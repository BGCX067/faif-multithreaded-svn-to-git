#ifndef FAIF_COMMAND_DESC_H
#define FAIF_COMMAND_DESC_H

#include <boost/smart_ptr.hpp>

namespace faif {

    /** \brief active object design pattern based on boost::thread
     */
    namespace actobj {

		/** Command unique ID indentifier */
		typedef long CommandID;

		/** \brief the descriptor of command */
		struct CommandDesc {

			/** \brief available states of Command
				NONE - command created, but not put in activation queue
				QUEUED - command is waiting for execusion at the activation queue
				PENDING - command is being executed now
				INTERRUPTED - command execution has been interrupted
				EXCEPTION - during command execution an exception was catched
				DONE - command code has been done successfully
			*/
			enum State { NONE, QUEUED, PENDING, INTERRUPTED, EXCEPTION, DONE };

			CommandID id_; //!< command unique ID
			State state_; //!< command state
			double progress_; //!< command progress

        CommandDesc() : id_(0), state_(NONE), progress_(0.0) {}
		};

		/** forward declaration */
		class Command;

		/** abstract class of command observer */
		class CommandObserver {
		public:
			CommandObserver() { }
			/** new progress */
			virtual void notifyProgress(const Command&, double progress) = 0;

			/** next step */
			virtual void notifyStep(const Command&) = 0;

			/** change command state */
			virtual void notifyState(const Command&, CommandDesc::State state) = 0;

			virtual ~CommandObserver() { }
		};

		/** smart pointer to observer objects */
		typedef boost::shared_ptr<CommandObserver> PCommandObserver;
    } //namespace actobj

} //namespace faif

#endif //FAIF_COMMAND_DESC_H
