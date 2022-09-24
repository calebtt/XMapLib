using System;
using System.CodeDom;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    /* If the file is a bit long, try using the outlining feature of VS to collapse the member functions (methods) to definitions.
     right click -> outlining -> collapse to definitions */
    public partial class Form1 : Form
    {
        private XMapLibWrapper _mapper;
        private XMapLibStickMap _currentXMapLibStick = XMapLibStickMap.Right;
        private List<KeymapPreset> _presets = new();
        private List<XMapLibKeymap> _currentKeymaps = new();
        public Form1()
        {
            InitializeComponent();
            _mapper = new XMapLibWrapper();
            _mapper.SetMouseStick(_currentXMapLibStick);
            UpdateMouseSensitivityTrackbar();
            UpdateControllerConnectedButton();
            UpdateMouseSensitivityButton();
            UpdateIsMouseRunning();
            InitBackgroundWorker();
            InitPresetButtons();
            UpdateMapStringBox();
            InitDataGridView();
            tabControl1.SelectedIndexChanged += TabControl1_SelectedIndexChanged;
            this.FormClosing += OnFormClosing;
        }

        private void OnFormClosing(object? sender, FormClosingEventArgs e)
        {
            // Explicitly stops the processing for keyboard and mouse, as well as
            // stops the running static thread before program termination.
            this._mapper.StopBoth();
        }

        private void btnSensitivityIndicator_Click(object sender, EventArgs e)
        {

        }
        /// <summary>Event handler for mouse movement processing enable/disable.</summary>
        private void btnMouseProcessing_Click(object sender, EventArgs e)
        {
            //toggle processing
            if (_mapper.IsMouseRunning())
            {
                _mapper.StopBoth();
                btnMouseProcessing.Text = MsgStartMouse;
            }
            else
            {
                _mapper.StartBoth();
                btnMouseProcessing.Text = MsgStopMouse;
            }
        }
        /// <summary>Event handler for controller thumbstick select button</summary>
        private void btnStick_Click(object sender, EventArgs e)
        {
            _currentXMapLibStick = _currentXMapLibStick.Next();
            btnStick.Text = _currentXMapLibStick.ToString() + " " + MsgStick;
            _mapper.SetMouseStick(_currentXMapLibStick);
            UpdateIsMouseRunning();
        }
        /// <summary> Background GUI thread to update certain statuses, such as is the controller connected. </summary>
        private void bgWorkThread_DoWork(object sender, DoWorkEventArgs e)
        {
            static void ShowErrorMessage(string msg)
            {
                StringBuilder sb = new();
                sb.AppendFormat("Error in {0}, unable to get handle to Sync Context. {1}", nameof(bgWorkThread_DoWork), msg);
                MessageBox.Show(sb.ToString());
            }
            if (e.Argument is SynchronizationContext sc)
            {
                while (!bgWorkThread.CancellationPending)
                {
                    sc.Post(delegate (object? state) { UpdateControllerConnectedButton(); }, null);
                    sc.Post(delegate (object? state) { UpdateMapStringBox(); }, null);
                    Thread.Sleep(DelayRedrawMs);
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
        /// <summary>Event handler for mouse sensitivity trackbar.</summary>
        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            int val = trackBar1.Value;
            _mapper.SetMouseSensitivity(val);
            UpdateMouseSensitivityButton();
            UpdateIsMouseRunning();
        }
        /// <summary>Event handler for preset buttons</summary>
        private void ButtonForPresetSection_Click(object? sender, EventArgs e)
        {
            if (sender is Button b)
            {
                bool isButtonTextSelected = KeymapPresetOperations.IsButtonTextSelected(b);
                //if is selected, do nothing, otherwise..
                if (!isButtonTextSelected)
                {
                    //set each button to not selected.
                    foreach (var p in _presets)
                    {
                        if (KeymapPresetOperations.IsButtonTextSelected(p.ButtonForPresetSection))
                            KeymapPresetOperations.ChangeButtonTextForSelected(p.ButtonForPresetSection, false);
                    }
                    //select sending button
                    KeymapPresetOperations.ChangeButtonTextForSelected(b, !isButtonTextSelected);
                    //find button in preset list, change keymaps over.
                    _mapper.ClearKeyMaps();
                    foreach (var p in _presets)
                    {
                        if (p.ButtonForPresetSection == b)
                        {
                            _mapper.AddKeymaps(p.Keymaps);
                            UpdateKeymapDatagrid(p.Keymaps);
                        }
                    }
                }
            }
        }
        /// <summary>Event handler for datagridview cell item value changed.</summary>
        private void DataGridView1_CellValueChanged(object sender, DataGridViewCellEventArgs e)
        {
            ControllerButtons? FindControllerButtonByName(string name)
            {
                var valueList = Enum.GetValues(typeof(ControllerButtons)).Cast<ControllerButtons>();
                foreach (var value in valueList)
                {
                    if (name == value.ToString())
                        return value;
                }
                return null;
            }
            Keys? FindKeyByName(string name)
            {
                var valueList = Enum.GetValues(typeof(Keys)).Cast<Keys>();
                foreach (var value in valueList)
                {
                    if (name == value.ToString())
                        return value;
                }
                return null;
            }
            //get row and column
            int row = e.RowIndex;
            int col = e.ColumnIndex;
            var workedWithMap = _currentKeymaps[row]; //they are a struct so it's a copy
            switch (col)
            {
                case 0:
                //mapped from element
                case 1:
                    //mapped from enum element
                    if (this.dataGridView1[col, row].EditedFormattedValue is string enumAsString)
                    {
                        ControllerButtons? elem = FindControllerButtonByName(enumAsString);
                        if (elem != null)
                        {
                            workedWithMap.VkMappedFrom = (int)elem;
                            _currentKeymaps[row] = workedWithMap;
                        }
                    }
                    break;
                case 2:
                //mapped to element
                case 3:
                    //mapped to enum element
                    if (this.dataGridView1[col, row].EditedFormattedValue is string keyEnumAsString)
                    {
                        Keys? elem = FindKeyByName(keyEnumAsString);
                        if (elem != null)
                        {
                            workedWithMap.VkMappedTo = (int)elem;
                            _currentKeymaps[row] = workedWithMap;
                        }
                    }
                    break;
                case 4:
                    //bool uses repeat checkbox
                    workedWithMap.UsesRepeatBehavior = !workedWithMap.UsesRepeatBehavior;
                    _currentKeymaps[row] = workedWithMap;
                    break;
                default:
                    throw new NotImplementedException("Exception in " + MethodBase.GetCurrentMethod()?.Name + ", a column was selected that is not a part of this handler!");
            }
            dataGridView1.Refresh();
        }
        /// <summary>Event raised when clicking a tab page on the tabcontrol.</summary>
        private void TabControl1_SelectedIndexChanged(object? sender, EventArgs e)
        {
            if (tabControl1.SelectedIndex == 0)
            {
                _mapper.ClearKeyMaps();
                if (!_mapper.AddKeymaps(_currentKeymaps))
                {
                    MessageBox.Show(ErrUpdatingMaps, ErrUpdatingMaps, MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
            }
        }

    }
}
