using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace scada.Models
{
    public class CorModel
    {
        public string Nome { get; set; }
        public float R { get; set; }
        public float G { get; set; }
        public float B { get; set; }

        public CorModel(string nome, float r, float g, float b)
        {
            Nome = nome;
            R = r;
            G = g;
            B = b;
        }
    }
}
