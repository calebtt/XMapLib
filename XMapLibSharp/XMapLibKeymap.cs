using System;
using System.Text;
using System.Windows.Forms;

namespace XMapLibSharp
{
    public struct XMapLibKeymap
    {
        private int _vkMappedTo;
        private int _vkMappedFrom;

        public int VkMappedFrom
        {
            get => _vkMappedFrom;
            set
            {
                _vkMappedFrom = value;
                string? result = Enum.GetName((ControllerButtons)_vkMappedFrom);
                if (result != null)
                    VkMappedFromAka = result;
            }
        }
        public string VkMappedFromAka { get; set; }
        public int VkMappedTo
        {
            get => _vkMappedTo;
            set
            {
                _vkMappedTo = value;
                string? result = Enum.GetName((Keys)_vkMappedTo);
                if (result != null)
                    VkMappedToAka = result;
            }
        }
        public string VkMappedToAka {  get; set; }
        public bool UsesRepeatBehavior { get; set; }

        public override string ToString()
        {
            StringBuilder sb = new();
            sb.Append(nameof(VkMappedFrom) + ":" + VkMappedFrom + " ");
            sb.Append(nameof(VkMappedFromAka) + ":" + VkMappedFromAka + " ");
            sb.Append(nameof(VkMappedTo) + ":" + VkMappedTo + " ");
            sb.Append(nameof(VkMappedToAka) + ":" + VkMappedToAka + " ");
            sb.Append(nameof(UsesRepeatBehavior) + ":" + UsesRepeatBehavior);
            return sb.ToString();
        }
    }
}
