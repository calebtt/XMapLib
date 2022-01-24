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
        const string PRESET_BROWSING = "Browsing";
        const string PRESET_GAMING = "Gaming";
        const string PRESET_MOVIE = "Movie";
        const string MSG_PRESET_SELECTED = "\n*SELECTED*";
        public static void ChangeButtonTextForSelected(Button prButton, bool makeSelected)
        {
            if (makeSelected)
            {
                if(!IsButtonTextSelected(prButton))
                    prButton.Text += MSG_PRESET_SELECTED;
            }
            else
            {
                if (IsButtonTextSelected(prButton))
                {
                    string temp = prButton.Text;
                    int index = temp.IndexOf(MSG_PRESET_SELECTED, StringComparison.Ordinal);
                    prButton.Text = temp[..index];
                }
            }
        }

        public static bool IsButtonTextSelected(Button prButton)
        {
            string temp = prButton.Text;
            int index = temp.IndexOf(MSG_PRESET_SELECTED, StringComparison.Ordinal);
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
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PRESET_BROWSING), KeymapName = PRESET_BROWSING, Keymaps = BuildPresetBrowsing()});
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PRESET_GAMING), KeymapName = PRESET_BROWSING, Keymaps = BuildPresetGaming() });
            presetButtons.Add(new() { ButtonForPresetSection = BuildPresetButton(PRESET_MOVIE), KeymapName = PRESET_BROWSING, Keymaps = BuildPresetMovie() });
            return presetButtons;
        }
        public static List<XMapLibKeymap> BuildPresetBrowsing()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_DOWN, VKMappedTo = (int) Keys.Down },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_UP, VKMappedTo = (int) Keys.Up },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_LEFT, VKMappedTo = (int) Keys.Left },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_RIGHT, VKMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTRIGGER, VKMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_RTRIGGER, VKMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UP, VKMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_LEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWN, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_RIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPLEFT, VKMappedTo = (int)Keys.W},
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPLEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPRIGHT, VKMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPRIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNLEFT, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNLEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNRIGHT, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNRIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LSHOULDER, VKMappedTo = (int)Keys.BrowserBack },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_RSHOULDER, VKMappedTo = (int)Keys.BrowserForward },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_A, VKMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_B, VKMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_X, VKMappedTo = (int)Keys.R }
            };
            return mp;
        }
        public static List<XMapLibKeymap> BuildPresetGaming()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_DOWN, VKMappedTo = (int)Keys.Down },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_UP, VKMappedTo = (int)Keys.Up },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_LEFT, VKMappedTo = (int)Keys.Left },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_RIGHT, VKMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTRIGGER, VKMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_RTRIGGER, VKMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UP, VKMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_LEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWN, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_RIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPLEFT, VKMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPLEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPRIGHT, VKMappedTo = (int)Keys.W },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UPRIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNLEFT, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNLEFT, VKMappedTo = (int)Keys.A },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNRIGHT, VKMappedTo = (int)Keys.S },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWNRIGHT, VKMappedTo = (int)Keys.D },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_A, VKMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_B, VKMappedTo = (int)Keys.F },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_Y, VKMappedTo = (int)Keys.G},
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_RTHUMB_PRESS, VKMappedTo = (int)Keys.C },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_PRESS, VKMappedTo = (int)Keys.LShiftKey },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_START, VKMappedTo = (int)Keys.Escape },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LSHOULDER, VKMappedTo = (int)Keys.Q },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_RSHOULDER, VKMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_X, VKMappedTo = (int)Keys.R }
            };
            return mp;
        }
        public static List<XMapLibKeymap> BuildPresetMovie()
        {
            List<XMapLibKeymap> mp = new()
            {
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_DOWN, VKMappedTo = (int)Keys.VolumeDown },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_UP, VKMappedTo = (int)Keys.VolumeUp },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_LEFT, VKMappedTo = (int)Keys.Left },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_RIGHT, VKMappedTo = (int)Keys.Right },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTRIGGER, VKMappedTo = (int)Keys.RButton },
                new() { UsesRepeatBehavior = false, VKMappedFrom = (int)ControllerButtons.VK_PAD_RTRIGGER, VKMappedTo = (int)Keys.LButton },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_UP, VKMappedTo = (int)Keys.PageUp },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_LEFT, VKMappedTo = (int)Keys.BrowserBack },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_DOWN, VKMappedTo = (int)Keys.PageDown},
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LTHUMB_RIGHT, VKMappedTo = (int)Keys.BrowserForward },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_LSHOULDER, VKMappedTo = (int)Keys.MediaPlayPause },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_RSHOULDER, VKMappedTo = (int)Keys.VolumeMute },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_A, VKMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_B, VKMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_X, VKMappedTo = (int)Keys.R }
            };
            return mp;
        }
    }
}
