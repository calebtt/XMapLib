﻿using System;
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
        //private const string ERR_DLL_NOT_FOUND = "XMapLib.dll not found.";
        private const string ERR_NOT_RUNNING = "Started, but not running.";
        private const string MSG_START_MOUSE = "Start Mouse Processing";
        private const string MSG_STOP_MOUSE = "Stop Mouse Processing";
        private const string MSG_STICK = "Stick";
        private const string MSG_NOCONTROLLER = "Not Connected";
        private const string MSG_CONTROLLER = "Connected";
        private const string MSG_SENSMAX = "/100";
        private const string MSG_SENSBEGIN = "Sensitivity: ";
        private XMapLibWrapper mapper;
        private StickMap currentStick = StickMap.RIGHT;
        public Form1()
        {
            InitializeComponent();
            mapper = new XMapLibWrapper();
            mapper.SetMouseStick(currentStick);
            UpdateMouseSensitivityTrackbar();
            UpdateControllerConnectedButton();
            UpdateMouseSensitivityButton();
        }
        private void UpdateMouseSensitivityTrackbar()
        {
            trackBar1.Value = mapper.GetMouseSensitivity();
        }
        private void UpdateControllerConnectedButton()
        {
            bool isConnected = mapper.IsControllerConnected();
            button2.Text = isConnected ? MSG_CONTROLLER : MSG_NOCONTROLLER;
        }
        private void UpdateMouseSensitivityButton()
        {
            btnSensitivityIndicator.Text = MSG_SENSBEGIN + " " + mapper.GetMouseSensitivity().ToString() + MSG_SENSMAX;
        }
        private void DoErrorMessage(string msg)
        {
            MessageBox.Show(msg);
        }
        private void btnSensitivityIndicator_Click(object sender, EventArgs e)
        {

        }
        private void btnMouseProcessing_Click(object sender, EventArgs e)
        {
            //toggle processing
            if (mapper.IsMouseRunning())
            {
                mapper.StopMouse();
                btnMouseProcessing.Text = MSG_START_MOUSE;
            }
            else
            {
                mapper.InitMouse();
                btnMouseProcessing.Text = MSG_STOP_MOUSE;
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
