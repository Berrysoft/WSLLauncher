using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace Launcher
{
    internal enum WSL_DISTRIBUTION_FLAGS
    {
        NONE = 0x0,
        ENABLE_INTEROP = 0x1,
        APPEND_NT_PATH = 0x2,
        ENABLE_DRIVE_MOUNTING = 0x4,
        VALID = ENABLE_INTEROP | APPEND_NT_PATH | ENABLE_DRIVE_MOUNTING,
        DEFAULT = ENABLE_INTEROP | APPEND_NT_PATH | ENABLE_DRIVE_MOUNTING
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SECURITY_ATTRIBUTES
    {
        public int nLength;
        public IntPtr lpSecurityDescriptor;
        [MarshalAs(UnmanagedType.Bool)]
        public bool bInheritHandle;
    }

    internal static class NativeApi
    {
        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool WslIsDistributionRegistered(string distributionName);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslRegisterDistribution(string distributionName, string tarGzFilename);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslUnregisterDistribution(string distributionName);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslConfigureDistribution(string distributionName, uint defaultUID, WSL_DISTRIBUTION_FLAGS wslDistributionFlags);

        //[DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        //public static extern void WslGetDistributionConfiguration(string distributionName, out uint distributionVersion, out uint defaultUID, out WSL_DISTRIBUTION_FLAGS wslDistributionFlags, [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPWStr, SizeParamIndex = 5)] out string[] defaultEnvironmentVariables, out uint defaultEnvironmentVariableCount);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslLaunchInteractive(string distributionName, string? command, [MarshalAs(UnmanagedType.Bool)] bool useCurrentWorkingDirectory, out uint exitCode);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslLaunch(string distributionName, string? command, [MarshalAs(UnmanagedType.Bool)] bool useCurrentWorkingDirectory, SafeFileHandle stdIn, SafeFileHandle stdOut, SafeFileHandle stdErr, out SafeProcessHandle process);

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool CreatePipe(out SafeFileHandle hReadPipe, out SafeFileHandle hWritePipe, ref SECURITY_ATTRIBUTES lpPipeAttributes, uint nSize);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern SafeFileHandle GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern uint WaitForSingleObject(SafeHandle hHandle, uint dwMilliseconds = uint.MaxValue);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GetExitCodeProcess(SafeHandle hProcess, out uint lpExitCode);
    }
}
