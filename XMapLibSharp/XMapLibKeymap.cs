using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Permissions;
using System.Text;
using System.Threading.Tasks;

namespace XMapLibSharp
{
    public struct XMapLibKeymap
    {
        public int VKMappedFrom { get; set; }
        public int VKMappedTo { get; set; }
        public char? VKMappedToAKA {  get; set; }
        public bool UsesRepeatBehavior { get; set; }
    }
}
