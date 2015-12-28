#ifndef FAIF_OSTREAM_COMMAND_OBSERVER_H
#define FAIF_OSTREAM_COMMAND_OBSERVER_H

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
//msvc9.0 garbage warnings for boost::date_time
#pragma warning(disable:4244)
#pragma warning(disable:4512)
#endif

#include <ostream>
#include <boost/thread.hpp>

#include "CommandDesc.h"

namespace faif {
    namespace actobj {
        /** simple observer, send notification to ostream */
        class OstreamCommandObserver : public CommandObserver {
        public:
        OstreamCommandObserver(std::ostream& out) : out_(out) {}
            virtual ~OstreamCommandObserver() {}

            /** new progress */
            virtual void notifyProgress(const Command&, double progress) {
                boost::mutex::scoped_lock scoped_lock(out_mutex);
                out_ << static_cast<int>(progress * 100) << '%' << std::endl;
            }

            /** next step */
            virtual void notifyStep(const Command&) {
				boost::mutex::scoped_lock scoped_lock(out_mutex);
				out_ << '.' << std::flush;
            }

            /** change command state */
            virtual void notifyState(const Command&, CommandDesc::State) {}
        private:
            OstreamCommandObserver& operator=(const OstreamCommandObserver&); //!< not allowed

            boost::mutex out_mutex;
            std::ostream& out_;
        };


    } //namespace actobj
} // namespace faif


#endif // FAIF_OSTREAM_COMMAND_OBSERVER_H

