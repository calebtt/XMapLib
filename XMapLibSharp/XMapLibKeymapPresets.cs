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
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = true,
                    VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_DOWN,
                    VKMappedTo = (int) Keys.Down
                },
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = true,
                    VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_UP,
                    VKMappedTo = (int) Keys.Up
                },
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = true,
                    VKMappedFrom = (int) ControllerButtons.VK_PAD_DPAD_LEFT,
                    VKMappedTo = (int) Keys.Left
                },
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = true,
                    VKMappedFrom = (int)ControllerButtons.VK_PAD_DPAD_RIGHT,
                    VKMappedTo = (int)Keys.Right
                },
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = false,
                    VKMappedFrom = (int)ControllerButtons.VK_PAD_LTRIGGER,
                    VKMappedTo = (int)Keys.RButton
                },
                new XMapLibKeymap()
                {
                    UsesRepeatBehavior = false,
                    VKMappedFrom = (int)ControllerButtons.VK_PAD_RTRIGGER,
                    VKMappedTo = (int)Keys.LButton
                }
            };
            return mp;
        }
    }
}
