using System;
using System.CodeDom;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public partial class Form1 : Form
    {
        private const int DELAY_REDRAW_MS = 1000; // a whole second
        private const string ERR_NOT_RUNNING = "Started, but not running.";
        private const string MSG_START_MOUSE = "Start Mouse Processing";
        private const string MSG_STOP_MOUSE = "Stop Mouse Processing";
        private const string MSG_STICK = "Stick";
        private const string MSG_NOCONTROLLER = "Not Connected";
        private const string MSG_CONTROLLER = "Connected";
        private const string MSG_SENSMAX = "/100";
        private const string MSG_SENSBEGIN = "Sensitivity: ";
        private readonly Color CLR_INFO = Color.BurlyWood;
        private readonly Color CLR_NORMAL = Color.DarkSeaGreen;
        private XMapLibWrapper mapper;
        private XMapLibStickMap currentXMapLibStick = XMapLibStickMap.RIGHT;
        public Form1()
        {
            InitializeComponent();
            mapper = new XMapLibWrapper();
            mapper.SetMouseStick(currentXMapLibStick);
            UpdateMouseSensitivityTrackbar();
            UpdateControllerConnectedButton();
            UpdateMouseSensitivityButton();
            UpdateIsMouseRunning();
            UpdateMapStringBox();
            InitBackgroundWorker();
        }

        private void InitBackgroundWorker()
        {
            bgWorkThread.RunWorkerAsync(SynchronizationContext.Current);
        }
        private void UpdateMouseSensitivityTrackbar()
        {
            trackBar1.Value = mapper.GetMouseSensitivity();
        }
        private void UpdateControllerConnectedButton()
        {
            bool isConnected = mapper.IsControllerConnected();
            button2.Text = isConnected ? MSG_CONTROLLER : MSG_NOCONTROLLER;
            button2.BackColor = isConnected ? CLR_NORMAL : CLR_INFO;
        }
        private void UpdateMouseSensitivityButton()
        {
            btnSensitivityIndicator.Text = MSG_SENSBEGIN + " " + mapper.GetMouseSensitivity().ToString() + MSG_SENSMAX;
        }
        private void UpdateIsMouseRunning()
        {
            bool isRunning = mapper.IsMouseRunning();
            btnMouseProcessing.Text = isRunning ? MSG_STOP_MOUSE : MSG_START_MOUSE;
        }

        private void UpdateMapStringBox()
        {
            string currentMaps = mapper.GetKeyMaps();
            tbxMapDetails.Text = currentMaps;
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
            currentXMapLibStick = currentXMapLibStick.Next();
            btnStick.Text = currentXMapLibStick.ToString() + " " + MSG_STICK;
            mapper.SetMouseStick(currentXMapLibStick);
            UpdateIsMouseRunning();
        }
        /// <summary>
        /// Background GUI thread to update certain statuses, such as is the controller connected.
        /// </summary>
        private void bgWorkThread_DoWork(object sender, DoWorkEventArgs e)
        {
            static void ShowErrorMessage(string msg)
            {
                StringBuilder sb = new();
                sb.AppendFormat("Error in {0}, unable to get handle to Sync Context. {1}", nameof(bgWorkThread_DoWork), msg);
                MessageBox.Show(sb.ToString());
            }
            if ((e.Argument as SynchronizationContext) != null)
            {
                SynchronizationContext sc = (SynchronizationContext)e.Argument;
                while (!bgWorkThread.CancellationPending)
                {
                    sc.Send(delegate (object? state) { UpdateControllerConnectedButton(); }, null);
                    Thread.Sleep(DELAY_REDRAW_MS);
                }
            }
            else if (e.Argument != null)
            {
                ShowErrorMessage("e.Argument is not a SynchronizationContext!");
            }
            else
            {
                ShowErrorMessage("e.Argument is null!");
            }
        }

        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            int val = trackBar1.Value;
            mapper.SetMouseSensitivity(val);
            UpdateMouseSensitivityButton();
            UpdateIsMouseRunning();
        }
    }
}
