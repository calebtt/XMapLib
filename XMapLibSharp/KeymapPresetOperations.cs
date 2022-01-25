using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    /// <summary>A somewhat over-encompassing class for use by the main GUI form to work with presets and their buttons.
    /// It should support common operations such as determining the selected status of a button, changing the selected status,
    /// as well as building the buttons to be added to the form. </summary>
    public static class KeymapPresetOperations
    {
        private const int Width = 175;
        const int Height = 70;
        const float FontSizePresetButton = 8.5f;
        const string PresetBrowsing = "Browsing";
        const string PresetGaming = "Gaming";
        const string PresetMovie = "Movie";
        const string MsgPresetSelected = "\n*SELECTED*";
        public static void ChangeButtonTextForSelected(Button prButton, bool makeSelected)
        {
            if (makeSelected)
            {
                if(!IsButtonTextSelected(prButton))
                    prButton.Text += MsgPresetSelected;
            }
            else
            {
                if (IsButtonTextSelected(prButton))
                {
                    string temp = prButton.Text;
                    int index = temp.IndexOf(MsgPresetSelected, StringComparison.Ordinal);
                    prButton.Text = temp[..index];
                }
            }
        }

        public static bool IsButtonTextSelected(Button prButton)
        {
            string temp = prButton.Text;
            int index = temp.IndexOf(MsgPresetSelected, StringComparison.Ordinal);
            return (index != -1);
        }

        //A strong association between these values is kind of nice to have.
        public static List<KeymapPreset> BuildPresetButtons()
        {
            static Button BuildPresetButton(string presetName)
            {
                Button tb1 = new();
                Font f = new(FontFamily.GenericMonospace, FontSizePresetButton);
                tb1.Text = "[" + presetName + "]";
                tb1.Font = f;
                tb1.BackColor = Color.Aquamarine;
                tb1.MaximumSize = new(Width, Height);
                tb1.MinimumSize = new(Width, Height);
                tb1.FlatStyle = FlatStyle.Flat;
                tb1.Visible = true;
                tb1.TextAlign = ContentAlignment.MiddleCenter;
                return tb1;
            }
            List<KeymapPreset> presetButtons = new();
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PresetBrowsing), KeymapName = PresetBrowsing, Keymaps = BuildPresetBrowsing()});
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PresetGaming), KeymapName = PresetBrowsing, Keymaps = BuildPresetGaming() });
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PresetMovie), KeymapName = PresetBrowsing, Keymaps = BuildPresetMovie() });
            return presetButtons;
        }
        public static List<XMapLibKeymap> BuildPresetBrowsing()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int) ControllerButtons.VkPadDpadDown, VkMappedTo = (int) Keys.Down },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int) ControllerButtons.VkPadDpadUp, VkMappedTo = (int) Keys.Up },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int) ControllerButtons.VkPadDpadLeft, VkMappedTo = (int) Keys.Left },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadRight, VkMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadLtrigger, VkMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadRtrigger, VkMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUp, VkMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbLeft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDown, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbRight, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpleft, VkMappedTo = (int)Keys.W},
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpleft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpright, VkMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpright, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownleft, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownleft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownright, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownright, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLshoulder, VkMappedTo = (int)Keys.BrowserBack },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadRshoulder, VkMappedTo = (int)Keys.BrowserForward },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadA, VkMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadB, VkMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadX, VkMappedTo = (int)Keys.R }
            };
            return mp;
        }
        public static List<XMapLibKeymap> BuildPresetGaming()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadDown, VkMappedTo = (int)Keys.Down },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadUp, VkMappedTo = (int)Keys.Up },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadLeft, VkMappedTo = (int)Keys.Left },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadRight, VkMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadLtrigger, VkMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadRtrigger, VkMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUp, VkMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbLeft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDown, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbRight, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpleft, VkMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpleft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpright, VkMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUpright, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownleft, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownleft, VkMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownright, VkMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDownright, VkMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadA, VkMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadB, VkMappedTo = (int)Keys.F },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadY, VkMappedTo = (int)Keys.G},
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadRthumbPress, VkMappedTo = (int)Keys.C },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbPress, VkMappedTo = (int)Keys.LShiftKey },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadStart, VkMappedTo = (int)Keys.Escape },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLshoulder, VkMappedTo = (int)Keys.Q },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadRshoulder, VkMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadX, VkMappedTo = (int)Keys.R }
            };
            return mp;
        }
        public static List<XMapLibKeymap> BuildPresetMovie()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadDown, VkMappedTo = (int)Keys.VolumeDown },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadUp, VkMappedTo = (int)Keys.VolumeUp },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadLeft, VkMappedTo = (int)Keys.Left },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadDpadRight, VkMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadLtrigger, VkMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VkMappedFrom = (int)ControllerButtons.VkPadRtrigger, VkMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbUp, VkMappedTo = (int)Keys.PageUp },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbLeft, VkMappedTo = (int)Keys.BrowserBack },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbDown, VkMappedTo = (int)Keys.PageDown},
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLthumbRight, VkMappedTo = (int)Keys.BrowserForward },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadLshoulder, VkMappedTo = (int)Keys.MediaPlayPause },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadRshoulder, VkMappedTo = (int)Keys.VolumeMute },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadA, VkMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadB, VkMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VkMappedFrom = (int)ControllerButtons.VkPadX, VkMappedTo = (int)Keys.R }
            };
            return mp;
        }
    }
}
