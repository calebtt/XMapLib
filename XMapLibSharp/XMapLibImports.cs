using System;
using System.Runtime.InteropServices;

namespace XMapLibSharp
{
    internal static class XMapLibImports
    {
        private const string DllName = "XMapLibDLL.dll";

        [DllImport(DllName)]
        public static extern void XMapLibInitBoth();
        [DllImport(DllName)]
        public static extern void XMapLibInitMouse();
        [DllImport(DllName)]
        public static extern void XMapLibStopMouse();
        [DllImport(DllName)]
        public static extern void XMapLibInitKeyboard();
        [DllImport(DllName)]
        public static extern void XMapLibStopKeyboard();
        [DllImport(DllName)]
        public static extern void XMapLibStopBoth();
        [DllImport(DllName)]
        public static extern bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat);
        [DllImport(DllName)]
        public static extern void XMapLibClearMaps();
        [DllImport(DllName, CharSet=CharSet.Ansi)]
        public static extern IntPtr XMapLibGetMaps();
        [DllImport(DllName)]
        public static extern bool XMapLibIsControllerConnected();
        [DllImport(DllName)]
        public static extern bool XMapLibIsMouseRunning();
        [DllImport(DllName)]
        public static extern bool XMapLibIsKeyboardRunning();
        [DllImport(DllName)]
        public static extern void XMapLibSetMouseStick(int whichStick);
        [DllImport(DllName)]
        public static extern bool XMapLibSetMouseSensitivity(int sens);
        [DllImport(DllName)]
        public static extern int XMapLibGetMouseSensitivity();
    }
}
