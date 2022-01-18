using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public static class XMapLibKeymapPresets
    {
        public static List<XMapLibKeymap> BuildPresetBrowsing()
        {
            //
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
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_A, VKMappedTo = (int)Keys.Space },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_B, VKMappedTo = (int)Keys.E },
                new() { UsesRepeatBehavior = true, VKMappedFrom = (int)ControllerButtons.VK_PAD_X, VKMappedTo = (int)Keys.R }
            };
            return mp;
        }
    }
}
