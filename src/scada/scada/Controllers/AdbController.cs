using System.Diagnostics;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.IO;
using System.Windows.Forms;
using System;

namespace scada.Controllers
{
    public class AdbController
    {
        private string _adbPath = Path.Combine(Application.StartupPath, "Resources", "ADB", "adb.exe");

        private async Task<string> RunCommand(string args)
        {
            if (!File.Exists(_adbPath))
            {
                return $"ERRO: adb.exe não encontrado em: {_adbPath}";
            }

            return await Task.Run(() => {
                Process startInfo = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = _adbPath,
                        Arguments = args,
                        RedirectStandardOutput = true,
                        UseShellExecute = false,
                        CreateNoWindow = true
                    }
                };
                startInfo.Start();
                string result = startInfo.StandardOutput.ReadToEnd().Trim();
                startInfo.WaitForExit();
                return result;
            });
        }

        public async Task<string> GetSerialNumber()
        {
            string output = await RunCommand("devices");
            string[] lines = output.Split(new[] { Environment.NewLine, "\n" }, StringSplitOptions.RemoveEmptyEntries);

            foreach (var line in lines)
            {
                if (line.Contains("List of devices") || string.IsNullOrWhiteSpace(line))
                    continue;

                var parts = line.Split(new[] { ' ', '\t' }, StringSplitOptions.RemoveEmptyEntries);

                if (parts.Length > 0)
                {
                    string id = parts[0];

                    if (line.ToLower().Contains("unauthorized"))
                    {
                        return "ERRO: Autorize o PC na tela do celular!";
                    }
                    return id; // Retorna RQCTC01Z2TK
                }
            }
            return "NENHUM DISPOSITIVO";
        }
    }
}