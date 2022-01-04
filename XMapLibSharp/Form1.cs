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
        private const string MSG_START_MOUSE = "Start Mouse Processing";
        private const string MSG_STOP_MOUSE = "Stop Mouse Processing";
        private const string MSG_STICK = "Stick";
        private XMapLibWrapper mapper;
        private StickMap currentStick = StickMap.RIGHT;
        public Form1()
        {
            InitializeComponent();
            mapper = new XMapLibWrapper();
            mapper.SetMouseStick(currentStick);
            trackBar1.Value = mapper.GetMouseSensitivity();
        }

        private void DoErrorMessage(string msg)
        {
            MessageBox.Show(msg);
        }
        //private void btnStart_Click(object sender, EventArgs e)
        //{

        //}
        private void btnStop_Click(object sender, EventArgs e)
        {
            mapper.StopMouse();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            //toggle processing
            if (mapper.IsMouseRunning())
            {
                mapper.StopMouse();
                button1.Text = MSG_START_MOUSE;
            }
            else
            {
                mapper.InitMouse();
                button1.Text = MSG_STOP_MOUSE;
            }
        }

        private void btnStick_Click(object sender, EventArgs e)
        {
            currentStick = currentStick.Next();
            btnStick.Text = currentStick.ToString() + " " + MSG_STICK;
            mapper.SetMouseStick(currentStick);
        }
    }
}
