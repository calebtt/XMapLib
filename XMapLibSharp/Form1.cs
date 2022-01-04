using System;
using System.CodeDom;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public partial class Form1 : Form
    {
        private const string ERR_DLL_NOT_FOUND = "XMapLib.dll not found.";
        private const string ERR_NOT_RUNNING = "Started, but not running.";
        private XMapLibWrapper mapper;
        public Form1()
        {
            InitializeComponent();
            mapper = new XMapLibWrapper();
        }

        private void DoErrorMessage(string msg)
        {
            MessageBox.Show(msg);
        }
        private void btnStart_Click(object sender, EventArgs e)
        {
            mapper.InitMouse();
            mapper.SetMouseStick(XMapLibWrapper.StickMap.RIGHT);
            if (!mapper.IsMouseRunning())
            {
                DoErrorMessage(ERR_NOT_RUNNING);
            }
        }
        private void btnStop_Click(object sender, EventArgs e)
        {
            mapper.StopMouse();
        }
    }
}
