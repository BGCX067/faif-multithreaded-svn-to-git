#ifndef FAIF_SHEDULER_H
#define FAIF_SHEDULER_H

#include <queue>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include "Command.h"

namespace faif {
    namespace actobj {

		/** the return class of read command */
		typedef boost::optional<PCommand> OptPCommand;

		/* The command queue used by scheduler. Helping class. */
		class CommandQueue : boost::noncopyable {
		public:
			/** constructor */
			CommandQueue() : quitFlag_(false), currentCommandId_(0L) {}
			~CommandQueue() {}

			/** read the command to execute. Suspend the thread if the queue is empty.
				Returns not empty optional if the queue is destroyed. */
			OptPCommand read() {
				boost::mutex::scoped_lock lock(m_);
				while(queue_.empty()) {
					cond.wait(lock); //wait if the empty queue

					if(quitFlag_)   //if resumed because the quitFlag is set
						return OptPCommand();
				}
				//here non-empty queue
				PCommand f = queue_.front();
				queue_.pop();
				return f;
			}

			/** write the command into queue, returns the new command ID */
			CommandID write(PCommand cmd) {
				//unique_lock<boost::mutex> lock(m_);
				//to samo ale krocej zapisane
				boost::mutex::scoped_lock lock(m_);

				queue_.push(cmd);
				CommandID command_id = ++currentCommandId_;
				cmd->setId(command_id);
				cmd->setState(CommandDesc::QUEUED);
				cond.notify_all();
				return command_id;
			}

			/** accessor */
			bool getQuitFlag() const {
				boost::mutex::scoped_lock lock(m_);
				return quitFlag_;
			}
			/** mutator */
			void setQuitFlag() {
				boost::mutex::scoped_lock lock(m_);
				quitFlag_ = true;
				cond.notify_all();  //notify all waiting thread that the quit flag is set
			}

			long generateCommandId() {
				boost::mutex::scoped_lock lock(m_);
				return ++currentCommandId_;
			}
		private:
			std::queue<PCommand> queue_;
			mutable boost::mutex m_;
			/** the variable to notify if the queue is not empty */
			boost::condition_variable cond;

			bool quitFlag_;

			/** the ID for the next command */
			CommandID currentCommandId_;
		};

        /**
         *  Sheduler, the singleton containing the execution queue, and thread pool.
         *  it executes commands (synchronically or asynchronically)
         */
        class Scheduler
        {
        public:
            /**
               singleton
            */
            static Scheduler& getInstance() {
                //TODO! Change the parameter, because now the thread pool has always the size=4
                static Scheduler instance(8);
                return instance;
            }

            /**
               synchronically executes the command. Immediately executes the command
               in the calling thread context, so after return the flag 'done' is always set to true
               \return  CommandId of command
            */
            CommandID executeSynchronously(PCommand cmd);

            /**
               asynchronically executes the command. Do not break execution of the calling thread.
               \return  CommandId of command
            */
            CommandID executeAsynchronously(PCommand cmd);

            /**
               asynchronically executes the command. Command is inserted into command queue,
               it waits till free working thread is found, then it is executed.
               The calling thread waits (on condition variable) untill the command is not finished.
               \return  CommandId of command
            */
            CommandID executeAsynchronouslyAndWait(PCommand cmd);


            /** signal to finish all threads after current command */
            void finishAll();

            /** wait for ending threads in pool */
            void join();
        private:
            /** the activation queue. Contains every commands executed asynchronically */
            CommandQueue queue_;

            /** the thread pool */
            boost::thread_group pool_;

            Scheduler(int num_thrd); //the constructor
            Scheduler(const Scheduler&); //noncopyable
            Scheduler& operator=(const Scheduler&); //noncopyable
            ~Scheduler();
        };
    } //namespace actobj
} //namespace faif

//-------------------------------------------------------------------------------------------
//
// implementation
//
//-------------------------------------------------------------------------------------------

namespace faif {
    namespace actobj {

        namespace {

            /** the function which run in thread in scheduler thread pools.
                The main loops check if there is any command in queue, if so it executes one of them.
                When quitFlag is set in queue finish the execution
            */
            void threadFunction(CommandQueue& queue) {
                while(!queue.getQuitFlag()) { //check if the thread should finish execution
                    OptPCommand cmd = queue.read(); //reads command from queue,
                    //it can block the thread until the queue is not empty or until the queue is destroyed

                    if(!cmd) break; //empty optional - finish the thread
                    (*cmd)->execute();
                }
            }
        } //namespace

        // struct Scheduler::Impl {
        //     /** the activation queue. Contains every commands executed asynchronically */
        //     CommandQueue queue_;

        //     /** the thread pool */
        //     boost::thread_group pool_;


        //     Impl(int num_thrd) {
        //         for(int i = 0; i < num_thrd; ++i)       //starts num_thrd threads
        //             pool_.create_thread(boost::bind(threadFunction, boost::ref(queue_) ));
        //     }

        //     void join() { pool_.join_all(); }

        //     void finishAll() {
        //         queue_.setQuitFlag();
        //         join();
        //     }

        //     ~Impl() { finishAll(); }
        // };



        //constructor
        inline Scheduler::Scheduler(int num_thrd) {
			for(int i = 0; i < num_thrd; ++i)       //starts num_thrd threads
				pool_.create_thread(boost::bind(threadFunction, boost::ref(queue_) ));
		}

        //descructor
        inline Scheduler::~Scheduler() {
			finishAll();
		}

        /**
           synchronically executes the command. Immediately executes the command
           in the calling thread context, so after return the flag 'done' is always set to true
        */
        inline CommandID Scheduler::executeSynchronously(PCommand cmd) {
            CommandID command_id = queue_.generateCommandId();
            cmd->setId( command_id );
            cmd->execute();
            return command_id;
        }


        /**
           asynchronically executes the command. Do not break execution of the calling thread.
        */
        inline CommandID Scheduler::executeAsynchronously(PCommand cmd) {
            return queue_.write(cmd);
        }

        namespace {

            struct FinishObserver : public CommandObserver {

                FinishObserver() :done(false) {}

                virtual void notifyProgress(const Command&, double) {}
                virtual void notifyStep(const Command&) {}
                virtual void notifyState(const Command&, CommandDesc::State s) {
                    if(s == CommandDesc::DONE || s == CommandDesc::INTERRUPTED || s == CommandDesc::EXCEPTION) {
                        boost::mutex::scoped_lock lock(mut);
                        done = true;
                        cond.notify_one();
                    }
                }
                boost::mutex mut;
				boost::condition_variable cond;
                bool done;
            };

        }


        /**
           asynchronically executes the command. Command is inserted into command queue,
           it waits till free working thread is found, then it is executed.
           The calling thread waits (on condition variable) untill the command is not finished.
        */
        inline CommandID Scheduler::executeAsynchronouslyAndWait(PCommand cmd) {
            FinishObserver* obs = new FinishObserver();
            cmd->attach( PCommandObserver( obs ) );

            CommandID command_id = executeAsynchronously(cmd);

            boost::mutex::scoped_lock lock(obs->mut);
            while( ! obs->done ) {
                obs->cond.wait( lock );
            }
            return command_id;
        }


        /** signal to finish all threads after current command */
        inline void Scheduler::finishAll() {
			queue_.setQuitFlag();
			join();
        }

        /** wait for ending threads in pool */
        inline void Scheduler::join() {
			pool_.join_all();
        }

    } //namespace actobj
} //namespace faif

#endif

