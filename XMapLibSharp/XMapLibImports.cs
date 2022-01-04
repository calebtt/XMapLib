using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace XMapLibSharp
{
    public class XMapLibImports
    {
        private const string DLL_NAME = "XMapLibDLL.dll";

        [DllImport(DLL_NAME)]
        public static extern void XMapLibInitBoth();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibInitMouse();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibStopMouse();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibInitKeyboard();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibStopKeyboard();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibStopBoth();
        [DllImport(DLL_NAME)]
        public static extern bool XMapLibAddMap(int vkSender, int vkMapping, bool bUsesRepeat);
        [DllImport(DLL_NAME)]
        public static extern void XMapLibClearMaps();
        [DllImport(DLL_NAME)]
        public static extern bool XMapLibIsControllerConnected();
        [DllImport(DLL_NAME)]
        public static extern bool XMapLibIsMouseRunning();
        [DllImport(DLL_NAME)]
        public static extern bool XMapLibIsKeyboardRunning();
        [DllImport(DLL_NAME)]
        public static extern void XMapLibSetMouseStick(int whichStick);
        [DllImport(DLL_NAME)]
        public static extern bool XMapLibSetMouseSensitivity(int sens);
        [DllImport(DLL_NAME)]
        public static extern int XMapLibGetMouseSensitivity();
    }
}
