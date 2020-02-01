using System;
using System.IO;
using Microsoft.Win32.SafeHandles;

namespace Launcher
{
    class Distribution
    {
        public string Name { get; set; }

        public Distribution(string name)
        {
            Name = name;
        }

        public bool Registered
        {
            get
            {
                return NativeApi.WslIsDistributionRegistered(Name);
            }
        }

        public uint LaunchInteractive(string? command, bool useCurrentWorkingDirectory)
        {
            try
            {
                NativeApi.WslLaunchInteractive(Name, command, useCurrentWorkingDirectory, out uint exitCode);
                return exitCode;
            }
            catch (Exception e)
            {
                Console.WriteLine("WslLaunchInteractive {0} failed with error: 0x{1:X}", command, e.HResult);
                throw;
            }
        }

        private SafeProcessHandle Launch(string? command, bool useCurrentWorkingDirectory, SafeFileHandle stdIn, SafeFileHandle stdOut, SafeFileHandle stdErr)
        {
            try
            {
                NativeApi.WslLaunch(Name, command, useCurrentWorkingDirectory, stdIn, stdOut, stdErr, out SafeProcessHandle process);
                return process;
            }
            catch (Exception e)
            {
                Console.WriteLine("WslLaunch {0} failed with error: 0x{1:X}", command, e.HResult);
                throw;
            }
        }

        private bool LaunchIgnoreReturn(string? command, bool useCurrentWorkingDirectory)
        {
            try
            {
                uint exitCode = LaunchInteractive(command, useCurrentWorkingDirectory);
                return exitCode == 0;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private void Register(string tarball)
        {
            try
            {
                if (!File.Exists(tarball)) throw new FileNotFoundException();
                NativeApi.WslRegisterDistribution(Name, tarball);
            }
            catch (Exception e)
            {
                Console.WriteLine("WslRegisterDistribution failed with error: 0x{0:X}", e.HResult);
                throw;
            }
        }

        private void Configure(uint defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags)
        {
            try
            {
                NativeApi.WslConfigureDistribution(Name, defaultUID, wslDistributionFlags);
            }
            catch (Exception e)
            {
                Console.WriteLine("WslConfigureDistribution failed with error: 0x{0:X}", e.HResult);
                throw;
            }
        }

        private (uint DefaultUid, WSL_DISTRIBUTION_FLAGS Flags) GetConfigure()
        {
            try
            {
                NativeApi.WslGetDistributionConfiguration(Name, out _, out uint uid, out WSL_DISTRIBUTION_FLAGS flags, out _, out _);
                return (uid, flags);
            }
            catch (Exception e)
            {
                Console.WriteLine("WslGetDistributionConfiguration failed with error: 0x{0:X}", e.HResult);
                throw;
            }
        }

        public DistributionConfig Config
        {
            get
            {
                var (uid, flags) = GetConfigure();
                return new DistributionConfig
                {
                    DefaultUid = uid,
                    AppendPath = (flags & WSL_DISTRIBUTION_FLAGS.APPEND_NT_PATH) != 0,
                    MountDrive = (flags & WSL_DISTRIBUTION_FLAGS.ENABLE_DRIVE_MOUNTING) != 0
                };
            }
            set
            {
                WSL_DISTRIBUTION_FLAGS flags = WSL_DISTRIBUTION_FLAGS.ENABLE_INTEROP;
                if (value.AppendPath) flags |= WSL_DISTRIBUTION_FLAGS.APPEND_NT_PATH;
                if (value.MountDrive) flags |= WSL_DISTRIBUTION_FLAGS.ENABLE_DRIVE_MOUNTING;
                Configure(value.DefaultUid, flags);
            }
        }

        private bool CreateUser(string username)
        {
            // Create the user account.
            if (!LaunchIgnoreReturn("/usr/sbin/useradd -m " + username, true))
            {
                return false;
            }

            // Add the user account to any relevant groups.
            if (!LaunchIgnoreReturn("/usr/sbin/usermod -aG adm,cdrom,sudo,dip,plugdev " + username, true))
            {
                // Delete the user if the group add command failed.
                LaunchIgnoreReturn("/usr/sbin/userdel -r " + username, true);
                return false;
            }

            return true;
        }

        public unsafe uint QueryUid(string username)
        {
            SECURITY_ATTRIBUTES sa = new SECURITY_ATTRIBUTES
            {
                nLength = sizeof(SECURITY_ATTRIBUTES),
                lpSecurityDescriptor = IntPtr.Zero,
                bInheritHandle = true
            };
            if (NativeApi.CreatePipe(out SafeFileHandle readPipe, out SafeFileHandle writePipe, ref sa, 0))
            {
                var child = Launch("/usr/bin/id -u " + username, true, NativeApi.GetStdHandle(NativeApi.STD_INPUT_HANDLE), writePipe, NativeApi.GetStdHandle(NativeApi.STD_ERROR_HANDLE));
                NativeApi.WaitForSingleObject(child);
                NativeApi.GetExitCodeProcess(child, out uint exitCode);
                if (exitCode != 0)
                {
                    throw new ArgumentException();
                }
                using (var readStream = new FileStream(readPipe, FileAccess.Read))
                using (var reader = new StreamReader(readStream))
                {
                    string? line = reader.ReadLine();
                    return line != null ? uint.Parse(line) : throw new ArgumentException();
                }
            }
            throw new IOException();
        }

        public void Install(string tarball, bool createUser)
        {
            // Register the distribution.
            Console.WriteLine("Installing, this may take a few minutes...");
            Register(tarball);

            // Delete /etc/resolv.conf to allow WSL to generate a version based on Windows networking information.
            LaunchInteractive("/bin/rm /etc/resolv.conf", true);

            // Create a user account.
            if (createUser)
            {
                Console.WriteLine("Please create a default UNIX user account. The username does not need to match your Windows username.");
                Console.WriteLine("For more information visit: https://aka.ms/wslusers");
                string username;
                do
                {
                    username = Console.ReadLine();
                    if (username.Length > 32) username = username.Substring(0, 32);
                } while (!CreateUser(username));

                // Set this user account as the default.
                Configure(QueryUid(username), WSL_DISTRIBUTION_FLAGS.DEFAULT);
            }
        }

        public void Uninstall()
        {
            try
            {
                NativeApi.WslUnregisterDistribution(Name);
            }
            catch (Exception e)
            {
                Console.WriteLine("WslUnregisterDistribution failed with error: 0x{0:X}", e.HResult);
                throw;
            }
        }
    }

    struct DistributionConfig
    {
        public uint DefaultUid;
        public bool AppendPath;
        public bool MountDrive;
    }
}
