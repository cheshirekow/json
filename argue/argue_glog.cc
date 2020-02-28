// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/argue_glog.h"

#include <glog/logging.h>
#include "argue/argue_macros.h"

namespace argue {

// Add glog options (normally exposed through gflags) to the parser
void AddGlogOptions(Parser* parser) {
  parser->AddArgument(
      "--log-to-stderr", &FLAGS_logtostderr,
      {/*.action_=*/"store_true", ARGUE_EMPTY6,
       /*.help_=*/
       "Set whether log messages go to stderr instead of logfiles"});

  parser->AddArgument(
      "--also-log-to-stderr", &FLAGS_alsologtostderr,
      {/*.action_=*/"store_true", ARGUE_EMPTY6,
       /*.help_=*/
       "Set whether log messages go to stderr in addition to logfiles."});

  parser->AddArgument(
      "--color-log-to-stderr", &FLAGS_colorlogtostderr,
      {/*.action_=*/"store_true", ARGUE_EMPTY6,
       /*.help_=*/
       "Set color messages logged to stderr (if supported by terminal)."});

  parser->AddArgument(
      "--stderr-threshold", &FLAGS_stderrthreshold,
      {ARGUE_EMPTY7, /*.help_ =*/
       "Copy log messages at or above this level to stderr in addition to "
       "logfiles. The numbers of severity levels INFO, WARNING, ERROR, and "
       "FATAL are 0, 1, 2, and 3, respectively."});

  parser->AddArgument(
      "--log-prefix", &FLAGS_log_prefix,
      {ARGUE_EMPTY7,
       /*.help_=*/"Set whether the log prefix should be prepended "
                  "to each line of output."});

  parser->AddArgument(
      "--min-log-level", &FLAGS_minloglevel,
      {ARGUE_EMPTY7,
       /*.help_=*/
       "Log messages at or above this level. Again, the numbers of "
       "severity levels INFO, WARNING, ERROR, and FATAL are 0, 1, 2, "
       "and 3, respectively."});

  parser->AddArgument(
      "--log-dir", &FLAGS_log_dir,
      {ARGUE_EMPTY7,
       /*.help_=*/"If specified, logfiles are written into this directory "
                  "instead of the default loggind directory."});

  parser->AddArgument(
      "-v", "--verbose", &FLAGS_v,
      {ARGUE_EMPTY7,
       /*.help_=*/"Show all VLOG(m) messages for m less or equal the value of "
                  "this flag. Overridable by --vmodule. See the section about "
                  "verbose logging for more detail."});
}

}  // namespace argue
