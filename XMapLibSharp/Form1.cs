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
    /* If the file is a bit long, try using the outlining feature of VS to collapse the member functions (methods) to definitions.
     right click -> outlining -> collapse to definitions */
    public partial class Form1 : Form
    {
        private const int DelayRedrawMs = 1000; // a whole second
        private const string ErrNotRunning = "Started, but not running.";
        private const string ErrPresetButtonStatus = "Error toggling preset button selected status.";
        private const string ErrUpdatingMaps = "Error updating maps!";
        private const string MsgStartMouse = "Start Mouse Processing";
        private const string MsgStopMouse = "Stop Mouse Processing";
        private const string MsgStick = "Stick";
        private const string MsgNocontroller = "Not Connected";
        private const string MsgController = "Connected";
        private const string MsgSensmax = "/100";
        private const string MsgSensbegin = "Sensitivity: ";
        private readonly Color _clrInfo = Color.BurlyWood;
        private readonly Color _clrNormal = Color.DarkSeaGreen;
        private readonly XMapLibWrapper _mapper;
        private XMapLibStickMap _currentXMapLibStick = XMapLibStickMap.Right;
        private List<KeymapPreset> _presets = new();
        private List<XMapLibKeymap> _currentKeymaps = new();
        private readonly string[] keyNames = Enum.GetNames(typeof(Keys));
        private readonly string[] buttonNames = Enum.GetNames(typeof(ControllerButtons));
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
        }
        /// <summary>Event raised when clicking a tab page on the tabcontrol.</summary>
        private void TabControl1_SelectedIndexChanged(object? sender, EventArgs e)
        {
            if (tabControl1.SelectedIndex == 0)
            {
                _mapper.ClearKeyMaps();
                if(!_mapper.AddKeymaps(_currentKeymaps))
                {
                    MessageBox.Show(ErrUpdatingMaps);
                }
            }
        }

        private void InitDataGridView()
        {
            dataGridView1.CellValueChanged += DataGridView1_CellValueChanged;
            DataGridViewTextBoxColumn col1 = new();
            DataGridViewComboBoxColumn col2 = new();
            DataGridViewTextBoxColumn col3 = new();
            DataGridViewComboBoxColumn col4 = new();
            DataGridViewCheckBoxColumn col5 = new();
            col1.Name = nameof(XMapLibKeymap.VkMappedFrom);
            col2.Name = nameof(XMapLibKeymap.VkMappedFromAka);
            col3.Name = nameof(XMapLibKeymap.VkMappedTo);
            col4.Name = nameof(XMapLibKeymap.VkMappedToAka);
            col5.Name = nameof(XMapLibKeymap.UsesRepeatBehavior);
            col1.DataPropertyName = nameof(XMapLibKeymap.VkMappedFrom);
            col2.DataPropertyName = nameof(XMapLibKeymap.VkMappedFromAka);
            col3.DataPropertyName = nameof(XMapLibKeymap.VkMappedTo);
            col4.DataPropertyName = nameof(XMapLibKeymap.VkMappedToAka);
            col5.DataPropertyName = nameof(XMapLibKeymap.UsesRepeatBehavior);
            col2.DisplayStyle = DataGridViewComboBoxDisplayStyle.Nothing;
            col4.DisplayStyle = DataGridViewComboBoxDisplayStyle.Nothing;
            col2.Items.AddRange(buttonNames);
            col4.Items.AddRange(keyNames);
            dataGridView1.Columns.Add(col1);
            dataGridView1.Columns.Add(col2);
            dataGridView1.Columns.Add(col3);
            dataGridView1.Columns.Add(col4);
            dataGridView1.Columns.Add(col5);
            dataGridView1.AutoGenerateColumns = false;
            this.dataGridView1.DataSource = _currentKeymaps;
        }

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
            var workedWithMap = _currentKeymaps[row];
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
                            workedWithMap.VkMappedFrom = (int) elem;
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
                            workedWithMap.VkMappedTo = (int) elem;
                            _currentKeymaps[row] = workedWithMap;
                        }
                    }
                    break;
                case 4:
                    //bool uses repeat checkbox
                    break;
                default:
                    throw new NotImplementedException();
            }
            dataGridView1.Refresh();
        }

        private void InitPresetButtons()
        {
            _presets = KeymapPresetOperations.BuildPresetButtons();
            //assumes at least one preset is built
            foreach (KeymapPreset pr in _presets)
            {
                this.flwPresetButtons.Controls.Add(pr.ButtonForPresetSection);
                pr.ButtonForPresetSection.Click += ButtonForPresetSection_Click;
            }
            //adding first element keymaps to mapper
            _mapper.ClearKeyMaps();
            _mapper.AddKeymaps(_presets[0].Keymaps);
            UpdateKeymapDatagrid(_presets[0].Keymaps);
            //activating first button in the list
            if (this.flwPresetButtons.Controls[0] is Button btn)
            {
                KeymapPresetOperations.ChangeButtonTextForSelected(btn, true);
            }
        }
        private void InitBackgroundWorker()
        {
            bgWorkThread.RunWorkerAsync(SynchronizationContext.Current);
        }
        private void UpdateKeymapDatagrid(List<XMapLibKeymap> newMaps)
        {
            dataGridView1.AutoGenerateColumns = false;
            _currentKeymaps = newMaps;
            dataGridView1.DataSource = newMaps;
        }
        private void UpdateMouseSensitivityTrackbar()
        {
            trackBar1.Value = _mapper.GetMouseSensitivity();
        }
        private void UpdateControllerConnectedButton()
        {
            bool isConnected = _mapper.IsControllerConnected();
            button2.Text = isConnected ? MsgController : MsgNocontroller;
            button2.BackColor = isConnected ? _clrNormal : _clrInfo;
        }
        private void UpdateMouseSensitivityButton()
        {
            btnSensitivityIndicator.Text = MsgSensbegin + " " + _mapper.GetMouseSensitivity().ToString() + MsgSensmax;
        }
        private void UpdateIsMouseRunning()
        {
            bool isRunning = _mapper.IsMouseRunning();
            btnMouseProcessing.Text = isRunning ? MsgStopMouse : MsgStartMouse;
        }
        private void UpdateMapStringBox()
        {
            _mapper.GetKeyMaps(out var currentMaps);
            tbxMapDetails.Text = currentMaps;
        }
        private void TogglePresetButtonStatus(Button prButton)
        {
            KeymapPresetOperations.ChangeButtonTextForSelected(prButton,
                !KeymapPresetOperations.IsButtonTextSelected(prButton));
        }
        private void btnSensitivityIndicator_Click(object sender, EventArgs e)
        {

        }
        private void btnMouseProcessing_Click(object sender, EventArgs e)
        {
            //toggle processing
            if (_mapper.IsMouseRunning())
            {
                _mapper.StopMouse();
                btnMouseProcessing.Text = MsgStartMouse;
            }
            else
            {
                _mapper.InitMouse();
                btnMouseProcessing.Text = MsgStopMouse;
            }
        }
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
        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            int val = trackBar1.Value;
            _mapper.SetMouseSensitivity(val);
            UpdateMouseSensitivityButton();
            UpdateIsMouseRunning();
        }
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
    }
}
