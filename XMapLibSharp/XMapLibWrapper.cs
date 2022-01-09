﻿using System;
using System.CodeDom;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO.MemoryMappedFiles;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace XMapLibSharp
{
    public class XMapLibWrapper
    {
        public struct XMapLibKeymap
        {
            public int VKMappedFrom { get; set; }
            public int VKMappedTo { get; set; }
            public bool UsesRepeatBehavior { get; set; }
        }
        public XMapLibWrapper()
        {
            XMapLibImports.XMapLibInitBoth();
        }
        ~XMapLibWrapper()
        {
            XMapLibImports.XMapLibStopBoth();
        }
        public void InitMouse()
        {
            XMapLibImports.XMapLibInitMouse();
        }
        public void StopMouse()
        {
            XMapLibImports.XMapLibStopMouse();
        }
        public void InitKeyboard()
        {
            XMapLibImports.XMapLibInitKeyboard();
        }
        public void StopKeyboard()
        {
            XMapLibImports.XMapLibStopKeyboard();
        }
        public void StopBoth()
        {
            XMapLibImports.XMapLibStopBoth();
        }
        public bool AddKeymap(XMapLibKeymap detail)
        {
            return XMapLibImports.XMapLibAddMap(detail.VKMappedFrom, detail.VKMappedTo, detail.UsesRepeatBehavior);
        }
        public void ClearKeyMaps()
        {
            XMapLibImports.XMapLibClearMaps();
        }
        public bool AddKeymaps(List<XMapLibKeymap> details)
        {
            bool[] results = new bool[details.Count];
            for (int i = 0; i < details.Count; i++)
            {
                results[i] = AddKeymap(details[i]);
            }
            int failures = results.Count((e) => { return !e;} );
            return failures == 0;
        }
        public string GetKeyMaps()
        {
            IntPtr p = XMapLibImports.XMapLibGetMaps();
            if (p != IntPtr.Zero)
            {
                string? retVal = Marshal.PtrToStringAnsi(p);
                if (retVal != null)
                {
                    return retVal;
                }
                else
                {
                    return "";
                }
            }
            return "";
        }
        public bool IsControllerConnected()
        {
            return XMapLibImports.XMapLibIsControllerConnected();
        }
        public bool IsMouseRunning()
        {
            return XMapLibImports.XMapLibIsMouseRunning();
        }
        public bool IsKeyboardRunning()
        {
            return XMapLibImports.XMapLibIsKeyboardRunning();
        }
        public void SetMouseStick(StickMap whichStick)
        {
            XMapLibImports.XMapLibSetMouseStick(Convert.ToInt32(whichStick));
        }
        public bool SetMouseSensitivity(int sens)
        {
            return XMapLibImports.XMapLibSetMouseSensitivity(sens);
        }

        public int GetMouseSensitivity()
        {
            return XMapLibImports.XMapLibGetMouseSensitivity();
        }
    }
}
