using System;
using System.Collections.Generic;
using System.Linq;
using CommandLine;

#pragma warning disable 8618

namespace Launcher
{
    class Program
    {
        public static Distribution Distro;

        static int Main(string[] args)
        {
            string name = AppDomain.CurrentDomain.FriendlyName;
            Distro = new Distribution(name);
            if (args.Length == 0)
            {
                return RunVerb(new RunVerb() { Login = true });
            }
            return Parser.Default.ParseArguments<InstallVerb, UninstallVerb, ConfigVerb, RunVerb>(args).MapResult<IVerb, int>(RunVerb, RunError);
        }

        static int RunVerb(IVerb verb)
        {
            try
            {
                return verb.Run();
            }
            catch (Exception e)
            {
                switch ((uint)e.HResult)
                {
                    case 0x800700B7: // ERROR_ALREADY_EXISTS
                        Console.WriteLine("The distribution installation has become corrupted.");
                        Console.WriteLine("Please select Reset from App Settings or uninstall and reinstall the app.");
                        break;
                    case 0x8007019E: // ERROR_LINUX_SUBSYSTEM_NOT_PRESENT
                        Console.WriteLine("The Windows Subsystem for Linux optional component is not enabled. Please enable it and try again.");
                        Console.WriteLine("See https://aka.ms/wslinstall for details.");
                        break;
                    default:
                        Console.WriteLine("Error: 0x{0:X} {1}", e.HResult, e.Message);
                        break;
                }
                return 1;
            }
        }

        static int RunError(IEnumerable<Error> errs)
        {
            if (errs.Any(e => e.Tag == ErrorType.HelpRequestedError || e.Tag == ErrorType.HelpVerbRequestedError || e.Tag == ErrorType.VersionRequestedError))
                return 0;
            else
                return 1;
        }
    }

    interface IVerb
    {
        int Run();
    }

    [Verb("install", HelpText = "Install the distribution and do not launch the shell when complete.")]
    class InstallVerb : IVerb
    {
        [Option("root", HelpText = "Do not create a user account and leave the default user set to root.")]
        public bool Root { get; set; }

        [Option("file", Default = "rootfs.tar.gz", HelpText = "The distro tarball.")]
        public string File { get; set; }

        public int Run()
        {
            Program.Distro.Install(File, Root);
            return 0;
        }
    }

    [Verb("uninstall", HelpText = "Uninstall the distribution.")]
    class UninstallVerb : IVerb
    {
        public int Run()
        {
            Program.Distro.Uninstall();
            return 0;
        }
    }

    [Verb("config", HelpText = "Configure settings for this distribution.")]
    class ConfigVerb : IVerb
    {
        [Option("default-user", HelpText = "Sets the default user. This must be an existing user.")]
        public string DefaultUser { get; set; }

        public int Run()
        {
            Program.Distro.SetDefaultUser(DefaultUser);
            return 0;
        }
    }

    [Verb("run", HelpText = "Run the provided command line in the current working directory.\nIf no command line is provided, the default shell is launched.")]
    class RunVerb : IVerb
    {
        [Value(0)]
        public IEnumerable<string>? Command { get; set; }

        [Option("login", HelpText = "Use current directory.")]
        public bool Login { get; set; }

        public int Run()
        {
            return (int)Program.Distro.LaunchInteractive(Command != null ? string.Join(' ', Command) : string.Empty, !Login);
        }
    }
}

#pragma warning restore 8618
