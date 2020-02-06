using System;
using System.Collections.Generic;
using CommandLine;

namespace Launcher
{
    class Program
    {
        public static Distribution Distro = new Distribution(AppDomain.CurrentDomain.FriendlyName);

        static int Main(string[] args)
        {
            if (args.Length == 0)
            {
                return RunVerb(new RunVerb() { Login = true });
            }
            return Parser.Default.ParseArguments<InstallVerb, UninstallVerb, ConfigVerb, GetVerb, RunVerb>(args).MapResult<IVerb, int>(RunVerb, RunError);
        }

        static int RunVerb(IVerb verb)
        {
            try
            {
                return verb.Run();
            }
            catch (Exception e)
            {
                switch (e.HResult)
                {
                    case NativeApi.HRESULT_ERROR_ALREADY_EXISTS:
                        Console.WriteLine("The distribution installation has become corrupted.");
                        Console.WriteLine("Please select Reset from App Settings or uninstall and reinstall the app.");
                        break;
                    case NativeApi.HRESULT_ERROR_LINUX_SUBSYSTEM_NOT_PRESENT:
                    case NativeApi.COR_E_DLLNOTFOUND:
                        Console.WriteLine("The Windows Subsystem for Linux optional component is not enabled. Please enable it and try again.");
                        Console.WriteLine("See https://aka.ms/wslinstall for details.");
                        break;
                    default:
#if DEBUG
                        Console.WriteLine(e);
#else
                        Console.WriteLine("Error: 0x{0:X} {1}", e.HResult, e.Message);
#endif
                        break;
                }
                return 1;
            }
        }

        static int RunError(IEnumerable<Error> errs)
        {
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
        public string File { get; set; } = "rootfs.tar.gz";

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
        [Option("default-user", Group = "config", SetName = "username", HelpText = "Sets the default user. This must be an existing user.")]
        public string? DefaultUser { get; set; }

        [Option("default-uid", Group = "config", SetName = "uid", HelpText = "Sets the default user with uid. This must be an existing user.")]
        public uint? DefaultUid { get; set; }

        [Option("interop", Group = "config", HelpText = "Switch of interop with Windows processes.")]
        public bool? Interop { get; set; }

        [Option("append-path", Group = "config", HelpText = "Switch of Append Windows PATH to $PATH.")]
        public bool? AppendPath { get; set; }

        [Option("mount-drive", Group = "config", HelpText = "Switch of Mount drives.")]
        public bool? MountDrive { get; set; }

        public int Run()
        {
            var config = Program.Distro.Config;
            if (DefaultUser != null)
                config.DefaultUid = Program.Distro.QueryUid(DefaultUser);
            else if (DefaultUid != null)
                config.DefaultUid = DefaultUid.Value;
            if (Interop != null)
                config.Interop = Interop.Value;
            if (AppendPath != null)
                config.AppendPath = AppendPath.Value;
            if (MountDrive != null)
                config.MountDrive = MountDrive.Value;
            Program.Distro.Config = config;
            return 0;
        }
    }

    [Verb("get", HelpText = "Get settings for this distribution.")]
    class GetVerb : IVerb
    {
        [Option("user-uid", Group = "get", SetName = "useruid", HelpText = "Get the user uid by username.")]
        public string? UserUid { get; set; }

        [Option("default-uid", Group = "get", SetName = "uid", HelpText = "Get the default user uid in this distribution.")]
        public bool DefaultUid { get; set; }

        [Option("interop", Group = "get", SetName = "interop", HelpText = "Get status of interop with Windows processes.")]
        public bool Interop { get; set; }

        [Option("append-path", Group = "get", SetName = "append", HelpText = "Get status of Append Windows PATH to $PATH.")]
        public bool AppendPath { get; set; }

        [Option("mount-drive", Group = "get", SetName = "mount", HelpText = "Get status of Mount drives.")]
        public bool MountDrive { get; set; }

        [Option("default-env", Group = "get", SetName = "env", HelpText = "Get default environment variables.")]
        public bool DefaultEnv { get; set; }

        public int Run()
        {
            if (UserUid != null)
                Console.WriteLine(Program.Distro.QueryUid(UserUid));
            else
            {
                var config = Program.Distro.Config;
                if (DefaultUid)
                    Console.WriteLine(config.DefaultUid);
                else if (Interop)
                    Console.WriteLine(config.Interop);
                else if (AppendPath)
                    Console.WriteLine(config.AppendPath);
                else if (MountDrive)
                    Console.WriteLine(config.MountDrive);
                else if (DefaultEnv)
                {
                    foreach (string e in config.EnvVariables)
                    {
                        Console.WriteLine(e);
                    }
                }
            }
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
