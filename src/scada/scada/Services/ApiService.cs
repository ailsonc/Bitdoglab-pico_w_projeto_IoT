using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using scada.Models;

namespace scada.Services
{
    public class ApiService
    {
        private readonly HttpClient _httpClient;
        private readonly string _baseUrl;

        public ApiService(string baseUrl)
        {
            _httpClient = new HttpClient();
            _baseUrl = baseUrl.TrimEnd('/');
        }

        // GET: Busca a cor de referência para o modelo configurado
        public async Task<CorModel> GetConfiguracaoModelo(string modelo)
        {
            try
            {
                var response = await _httpClient.GetAsync($"{_baseUrl}/configuracao/{modelo}");
                if (!response.IsSuccessStatusCode) return null;

                var content = await response.Content.ReadAsStringAsync();
                var data = JsonConvert.DeserializeObject<dynamic>(content);

                return new CorModel(
                    (string)data.nome_cor,
                    (float)data.r_ref,
                    (float)data.g_ref,
                    (float)data.b_ref
                );
            }
            catch { return null; }
        }

        // POST: Salva o resultado final (PASS/FAIL) no banco de dados
        public async Task<bool> SalvarValidacao(string modelo, string serial, bool aprovado)
        {
            try
            {
                var payload = new
                {
                    modelo = modelo,
                    numero_serie = serial,
                    status = aprovado ? "PASS" : "FAIL" // Enum compatível com o MySQL
                };

                var json = JsonConvert.SerializeObject(payload);
                var content = new StringContent(json, Encoding.UTF8, "application/json");

                var response = await _httpClient.PostAsync($"{_baseUrl}/validacao", content);
                return response.IsSuccessStatusCode;
            }
            catch { return false; }
        }
    }
}