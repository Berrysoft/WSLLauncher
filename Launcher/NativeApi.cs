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

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslGetDistributionConfiguration(string distributionName, out uint distributionVersion, out uint defaultUID, out WSL_DISTRIBUTION_FLAGS wslDistributionFlags, [MarshalAs(UnmanagedType.LPArray, ArraySubType = UnmanagedType.LPStr, SizeParamIndex = 5)] out string[] defaultEnvironmentVariables, out uint defaultEnvironmentVariableCount);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslLaunchInteractive(string distributionName, string? command, [MarshalAs(UnmanagedType.Bool)] bool useCurrentWorkingDirectory, out uint exitCode);

        [DllImport("wslapi.dll", CharSet = CharSet.Unicode, ExactSpelling = true, PreserveSig = false)]
        public static extern void WslLaunch(string distributionName, string? command, [MarshalAs(UnmanagedType.Bool)] bool useCurrentWorkingDirectory, SafeHandle stdIn, SafeHandle stdOut, SafeHandle stdErr, out SafeProcessHandle process);

        public const int STD_INPUT_HANDLE = -10;
        public const int STD_OUTPUT_HANDLE = -11;
        public const int STD_ERROR_HANDLE = -12;

        [DllImport("kernel32.dll", ExactSpelling = true, SetLastError = true)]
        public static extern SafeFileHandle GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll", ExactSpelling = true, SetLastError = true)]
        public static extern uint WaitForSingleObject(SafeHandle hHandle, uint dwMilliseconds = uint.MaxValue);

        [DllImport("kernel32.dll", ExactSpelling = true, SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GetExitCodeProcess(SafeHandle hProcess, out uint lpExitCode);

        public const int HRESULT_ERROR_ALREADY_EXISTS = unchecked((int)0x800700B7);
        public const int HRESULT_ERROR_LINUX_SUBSYSTEM_NOT_PRESENT = unchecked((int)0x8007019E);
        public const int COR_E_DLLNOTFOUND = unchecked((int)0x80131524);
    }
}
