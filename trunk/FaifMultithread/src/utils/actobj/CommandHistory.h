#ifndef FAIF_COMMAND_HISTORY_H
#define FAIF_COMMAND_HISTORY_H

#include <map>

#include <boost/noncopyable.hpp>

#include "Command.h"
#include "Scheduler.h"

namespace faif {
    namespace actobj {
        /** the collection of commands, it can find the command by Id to give the command status
            or progress etc. It is synchronized associative memory (std::map)
        */
        class CommandHistory : boost::noncopyable {
        public:
            CommandHistory() {}
            ~CommandHistory() {}

            /** add the command to collection with given ID  */
            void insert(CommandID id, PCommand cmd) {
                boost::mutex::scoped_lock lock(m_);
                history_.insert( std::make_pair( id, cmd ) );
            }

            /** try to find the Command of CommandId. If there is no command it returns the null pointer */
            PCommand find(CommandID id) const {
                boost::mutex::scoped_lock lock(m_);
                std::map<CommandID, PCommand>::const_iterator i = history_.find(id);
                if(i != history_.end() )
                    return i->second;
                else
                    return PCommand(0L);
            }

            /** clear the CommandHistory */
            void clear() {
				history_.clear();
            }
        private:
            mutable boost::mutex m_;
            std::map<CommandID, PCommand> history_;

        };

        /** \brief find the command descriptor of given ID.
            If the command is not found in history returns default Descriptor (id=0, status=NONE)
        */
        inline CommandDesc findCommandDescriptor(const CommandHistory& history, CommandID id) {
			PCommand cmd = history.find(id);
			if(cmd.get() != 0L)
				return cmd->getDescriptor();
			else
				return CommandDesc();
		}

		/** helpers, first executes in Scheduler and insterts cmd to CommandHistory */
		inline CommandID executeAsynchronouslyAndRemember(CommandHistory& history, PCommand cmd) {
			Scheduler& scheduler = Scheduler::getInstance();
			CommandID id = scheduler.executeAsynchronously(cmd);
			history.insert(id, cmd);
			return id;
		}

		/** helpers, first executes in Scheduler and insterts cmd to CommandHistory */
		inline CommandID executeSynchronouslyAndRemember(CommandHistory& history, PCommand cmd) {
			Scheduler& scheduler = Scheduler::getInstance();
			CommandID id = scheduler.executeSynchronously(cmd);
			history.insert(id, cmd);
			return id;
		}

    } //namespace actobj
} //namespace faif

#endif //FAIF_COMMAND_HISTORY_H
