
namespace XMapLibSharp
{
    public static class XMapLibStickMapOperations
    {
        public static XMapLibStickMap Next(this XMapLibStickMap rhs)
        {
            return rhs == XMapLibStickMap.Left ? XMapLibStickMap.Neither : rhs + 1;
        }

        public static string ToString(this XMapLibStickMap rhs)
        {
            switch (rhs)
            {
                case XMapLibStickMap.Neither:
                    return "NEITHER";
                case XMapLibStickMap.Left:
                    return "LEFT";
                case XMapLibStickMap.Right:
                    return "RIGHT";
                default: 
                    return "ERROR";
            }
        }
    }
}
