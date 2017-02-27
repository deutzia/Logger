# Logger
Simple logging library.

BASIC USAGE:

Logger::Log() may be treated as a stream, which can be logged to. It adds newline character automatically.

Logger::InitializeProgress is a function for tracking progress of a loop. It returns an object, which has the count() method that should be run every loop iteration.

Logger::Enter is a function to signal entering new environment, for example a function or method.

Logger::Exit is a function to leave such an environment
