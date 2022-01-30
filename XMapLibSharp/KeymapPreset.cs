using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public class KeymapPreset
    {
        public Button ButtonForPresetSection { get; set; } = new Button();
        public string KeymapName { get; set; } = String.Empty;
        public List<XMapLibKeymap> Keymaps { get; set; } = new List<XMapLibKeymap>();
    }
}
