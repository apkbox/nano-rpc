// <summary> 
// Simple console logging facility with colorization support.
// </summary>
 
namespace Odgs.Common
{
    using System;

    /// <summary>
    /// Specifies logging verbosity level.
    /// </summary>
    public enum Verbosity
    {
        /// <summary>
        /// Do not display anything.
        /// </summary>
        Quiet = -1, 

        /// <summary>
        /// Error level (least verbose).
        /// </summary>
        Error = 0, 

        /// <summary>
        /// Warning level.
        /// </summary>
        Warning = 1, 

        /// <summary>
        /// Important messages.
        /// </summary>
        Important = 2, 

        /// <summary>
        /// Informational messages.
        /// </summary>
        Info = 3, 

        /// <summary>
        /// Diagnosting messages (most verbose).
        /// </summary>
        Diagnostic = 4, 

        /// <summary>
        /// Custom verbosity level.
        /// </summary>
        Verbose1 = 5, 

        /// <summary>
        /// Custom verbosity level.
        /// </summary>
        Verbose2 = 6, 

        /// <summary>
        /// Custom verbosity level.
        /// </summary>
        Verbose3 = 7, 

        /// <summary>
        /// Custom verbosity level.
        /// </summary>
        Verbose4 = 8, 

        /// <summary>
        /// Display all messages.
        /// </summary>
        All = 1000
    }

    /// <summary>
    /// Provides basic console logging facility with coloration support.
    /// </summary>
    public static class Logger
    {
        /// <summary>
        /// Initializes static members of the Logger class.
        /// </summary>
        /// <remarks>
        /// The default verbosity level is Info.
        /// </remarks>
        static Logger()
        {
            Level = Verbosity.Info;
        }

        /// <summary>
        /// Gets or sets current verbosity level.
        /// </summary>
        public static Verbosity Level { get; set; }

        /// <summary>
        /// Logs Info level message.
        /// </summary>
        /// <param name="message">
        /// Message format string.
        /// </param>
        /// <param name="args">
        /// Arguments to substitute to the message string.
        /// </param>
        public static void LogInfo(string message, params object[] args)
        {
            LogText(Verbosity.Info, message, args);
        }

        /// <summary>
        /// Logs Important level message.
        /// </summary>
        /// <param name="message">
        /// Message format string.
        /// </param>
        /// <param name="args">
        /// Arguments to substitute to the message string.
        /// </param>
        public static void LogImportant(string message, params object[] args)
        {
            LogText(Verbosity.Important, message, args);
        }

        /// <summary>
        /// Logs Warning level message.
        /// </summary>
        /// <param name="message">
        /// Message format string.
        /// </param>
        /// <param name="args">
        /// Arguments to substitute to the message string.
        /// </param>
        public static void LogWarning(string message, params object[] args)
        {
            LogText(Verbosity.Warning, message, args);
        }

        /// <summary>
        /// Logs Error level message.
        /// </summary>
        /// <param name="message">
        /// Message format string.
        /// </param>
        /// <param name="args">
        /// Arguments to substitute to the message string.
        /// </param>
        public static void LogError(string message, params object[] args)
        {
            LogText(Verbosity.Error, message, args);
        }

        /// <summary>
        /// Logs message at the specified verbosity level.
        /// </summary>
        /// <param name="level">
        /// Verbosity level.
        /// </param>
        /// <param name="message">
        /// Message format string.
        /// </param>
        /// <param name="args">
        /// Arguments to substitute to the message string.
        /// </param>
        public static void LogText(Verbosity level, string message, params object[] args)
        {
            if( level <= Level )
            {
                ConsoleColor prev = Console.ForegroundColor;

                string tag = string.Empty;
                switch( level )
                {
                    case Verbosity.Error:
                        tag = "error: ";
                        Console.ForegroundColor = ConsoleColor.Red;
                        break;

                    case Verbosity.Warning:
                        tag = "warning: ";
                        Console.ForegroundColor = ConsoleColor.Yellow;
                        break;

                    case Verbosity.Important:
                        tag = "info: ";
                        Console.ForegroundColor = ConsoleColor.White;
                        break;

                    case Verbosity.Info:
                        tag = "info: ";
                        Console.ForegroundColor = ConsoleColor.Gray;
                        break;

                    default:
                        Console.ForegroundColor = ConsoleColor.Gray;
                        break;
                }

                if( args.Length == 0 )
                {
                    Console.WriteLine(tag + message);
                }
                else
                {
                    Console.WriteLine(tag + message, args);
                }

                Console.ForegroundColor = prev;
            }
        }
    }
}


