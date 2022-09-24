using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    /// <summary>
    /// Operations partial class for Form1, contains the bulk of the main form logic.
    /// Init funcs, helper funcs, constant data members.
    /// </summary>
    public partial class Form1
    {
        private const int DelayRedrawMs = 1000; // a whole second
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
        //private XMapLibWrapper _mapper;
        //private XMapLibStickMap _currentXMapLibStick = XMapLibStickMap.Right;
        //private List<KeymapPreset> _presets = new();
        //private List<XMapLibKeymap> _currentKeymaps = new();
        private readonly string[] _keyNames = Enum.GetNames(typeof(Keys));
        private readonly string[] _buttonNames = Enum.GetNames(typeof(ControllerButtons));

        /// <summary>Build items for the datagridview.</summary>
        private void InitDataGridView()
        {
            dataGridView1.CellValueChanged += DataGridView1_CellValueChanged;
            dataGridView1.AutoGenerateColumns = false;
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
            col2.Items.AddRange(_buttonNames.ToArray<Object>());
            col4.Items.AddRange(_keyNames.ToArray<Object>());
            dataGridView1.Columns.Add(col1);
            dataGridView1.Columns.Add(col2);
            dataGridView1.Columns.Add(col3);
            dataGridView1.Columns.Add(col4);
            dataGridView1.Columns.Add(col5);
            this.dataGridView1.DataSource = _currentKeymaps;
        }
        /// <summary>Init preset buttons and add to the GUI</summary>
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
        /// <summary>Starts running the background thread that updates the GUI elements.</summary>
        private void InitBackgroundWorker()
        {
            bgWorkThread.RunWorkerAsync(SynchronizationContext.Current);
        }
        /// <summary>Helper to update the datagridview to a new List of keymaps.</summary>
        private void UpdateKeymapDatagrid(List<XMapLibKeymap> newMaps)
        {
            dataGridView1.AutoGenerateColumns = false;
            _currentKeymaps = newMaps;
            dataGridView1.DataSource = newMaps;
        }
        /// <summary>Updates mouse sensitivity trackbar value based on mapper's internal reported value.</summary>
        private void UpdateMouseSensitivityTrackbar()
        {
            trackBar1.Value = _mapper.GetMouseSensitivity();
        }
        /// <summary>Helper to update the status of the controller being connected.</summary>
        private void UpdateControllerConnectedButton()
        {
            bool isConnected = _mapper.IsControllerConnected();
            button2.Text = isConnected ? MsgController : MsgNocontroller;
            button2.BackColor = isConnected ? _clrNormal : _clrInfo;
        }
        /// <summary>Helper to update the sensitivity value displayed on the button.</summary>
        private void UpdateMouseSensitivityButton()
        {
            btnSensitivityIndicator.Text = MsgSensbegin + " " + _mapper.GetMouseSensitivity().ToString() + MsgSensmax;
        }
        /// <summary>Helper to update the status of the mouse movement processing based on the mapper's internal status and update the button text.</summary>
        private void UpdateIsMouseRunning()
        {
            bool isRunning = _mapper.IsMouseRunning();
            btnMouseProcessing.Text = isRunning ? MsgStopMouse : MsgStartMouse;
        }
        /// <summary>Function to update the map string box.</summary>
        private void UpdateMapStringBox()
        {
            var mapList = _mapper.GetKeyMaps(out var currentMaps);
            StringBuilder sb = new();
            sb.AppendFormat("Processing {0} key maps." + Environment.NewLine, mapList.Count);
            foreach (var km in mapList)
            {
                sb.AppendFormat("[From]:{0} [To]:{1}" + Environment.NewLine, km.VkMappedFromAka, km.VkMappedToAka);
            }

            if (sb.ToString() != tbxMapDetails.Text)
            {
                tbxMapDetails.Text = "";
                tbxMapDetails.Text = sb.ToString();
            }
        }
    }
}
