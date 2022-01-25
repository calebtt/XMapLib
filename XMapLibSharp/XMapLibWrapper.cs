using System;
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
        private const string TOK_STARTMAP = "[KeyboardKeyMap]";
        //private const string TOK_ENDMAP = "[/KeyboardKeyMap]";
        //private const string TOK_SENDING_VK = "SendingElementVK";
        //private const string TOK_MAPPEDTO_VK = "MappedToVK";
        //private const string TOK_USES_REPEAT = "UsesRepeat";
        private const string REGEX_WS_PATTERN = @"\s+";
        private const string VALUE_DELIMITER = ":";
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
            int failures = results.Count(e => !e );
            return failures == 0;
        }
        public List<XMapLibKeymap> GetKeyMaps(out string mapsAsString)
        {
            mapsAsString = String.Empty;
            IntPtr p = XMapLibImports.XMapLibGetMaps();
            if (p != IntPtr.Zero)
            {
                string? retVal = Marshal.PtrToStringAnsi(p);
                if (retVal != null)
                {
                    mapsAsString = new string(retVal);
                    //parse strings, build list
                    string removedWs = System.Text.RegularExpressions.Regex.Replace(retVal, REGEX_WS_PATTERN, " ");
                    List<string> tokens = new();
                    foreach (string s in removedWs.Split(' '))
                    {
                        string temp = s.Replace(VALUE_DELIMITER, " ");// replacing value delimiter from tokens
                        tokens.Add(temp); // adding tokens to list
                    }
                    List<XMapLibKeymap> outMaps = new();
                    try
                    {
                        for (int i = 0; i < tokens.Count; i++)
                        {
                            if (tokens[i] == TOK_STARTMAP)
                            {
                                string[] akaStrings = tokens[i + 3].Split();
                                XMapLibKeymap mp = new();
                                mp.VKMappedFrom = Int32.Parse(tokens[i + 1].Split()[1]);
                                mp.VKMappedTo = Int32.Parse(tokens[i + 2].Split()[1]);
                                if (akaStrings.Length > 1)
                                {
                                    mp.VKMappedToAKA = (akaStrings[1]);
                                }
                                else
                                {
                                    mp.VKMappedToAKA = "";
                                }
                                string[] repeatStrings = tokens[i+4].Split();
                                if(repeatStrings.Length >1)
                                    if (repeatStrings[1].Length > 0)
                                        mp.UsesRepeatBehavior = Boolean.Parse(repeatStrings[1]);
                                outMaps.Add(mp);
                            }
                        }
                    }
                    catch
                    {
                        return new List<XMapLibKeymap>();
                    }
                    return outMaps;
                }
            }

            return new List<XMapLibKeymap>();
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
        public void SetMouseStick(XMapLibStickMap whichXMapLibStick)
        {
            XMapLibImports.XMapLibSetMouseStick(Convert.ToInt32(whichXMapLibStick));
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
