void Main()
{
	var siteKey = "";
	var apiKey = "";

	var token = new MultipassToken {
		Id = new Guid (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
		Name = "Chris",
		Email = "test@example.com",
		TokenExpires = DateTime.UtcNow.AddMinutes (30),
	};
	
	var encoder = new MultipassEncryptor (siteKey, apiKey);
	
	JsonConvert.SerializeObject (token).Dump ();
	
	var encryptedToken = encoder.Encrypt (token);
	encryptedToken.ToBase64SafeForUrl ().Dump ();
}

public class MultipassEncryptor
{
	private readonly string siteKey;
	private readonly string apiKey;
	
	public MultipassEncryptor (string siteKey, string apiKey)
	{
		this.siteKey = siteKey;
		this.apiKey = apiKey;
	}
	
	public EncryptedMultipass Encrypt (MultipassToken token)
	{
		var context = new MultipassEncryptionContext (siteKey, apiKey, token);
		return context.Encrypt ();
	}
}

public class MultipassEncryptionContext
{
	private readonly string siteKey;
	private readonly string apiKey;
	private readonly MultipassToken token;
	
	private byte[] initializationVector = Encoding.UTF8.GetBytes ("OpenSSL for Ruby");
	private byte[] encryptionKey;
	private byte[] buffer;
	private EncryptedMultipass encryptedToken;
	

	public MultipassEncryptionContext (string siteKey, string apiKey, MultipassToken token)
	{
		this.siteKey = siteKey;
		this.apiKey = apiKey;
		this.token = token;
	}
	
	public EncryptedMultipass Encrypt ()
	{
		ConvertTokenToJson ();
		CreateEncryptionKey ();
		EncryptEncodedToken ();
		return encryptedToken;
	}
	
	private void CreateEncryptionKey ()
	{
		var intermediateKey = Encoding.UTF8.GetBytes (apiKey + siteKey);
		using (var sha1 = SHA1.Create ()) {
			intermediateKey = sha1.ComputeHash (intermediateKey);
		}
		
		encryptionKey = new byte [16];
		Array.Copy (intermediateKey, encryptionKey, 16);
	}
	
	private void ConvertTokenToJson ()
	{
		buffer = Encoding.UTF8.GetBytes (JsonConvert.SerializeObject (token, Newtonsoft.Json.Formatting.Indented));
	}
	
	private void EncryptEncodedToken ()
	{
		XOrFirstBlockWithIV ();
		
		using (var aes = Aes.Create ()) {
			aes.Mode = CipherMode.CBC;
			aes.Padding = PaddingMode.PKCS7;
			aes.KeySize = 128;
			aes.BlockSize = 128;
			aes.IV = initializationVector;
			aes.Key = encryptionKey;
			
			using (var memoryStream = new MemoryStream ()) {
				using (var encryptor = aes.CreateEncryptor ())
				using (var cryptoStream = new CryptoStream (memoryStream, encryptor, CryptoStreamMode.Write)) {
					cryptoStream.Write (buffer, 0, buffer.Length);
					cryptoStream.FlushFinalBlock ();
				}
				
				encryptedToken = new EncryptedMultipass (memoryStream.ToArray ());
			}
		}
	}
	
	private void XOrFirstBlockWithIV ()
	{
		for (int i = 0; i < 16; i++)
			buffer [i] ^= initializationVector [i];
	}
}

public class EncryptedMultipass
{
	private readonly byte[] raw;
	
	public EncryptedMultipass (byte[] raw)
	{
		this.raw = raw;
	}
	
	public string ToBase64SafeForUrl ()
	{
		var temp = ToBase64 ();
		return temp
			.TrimEnd ('=')
			.Replace ('+', '-')
			.Replace ('/', '_');
	}
	
	public string ToBase64 ()
	{
		return Convert.ToBase64String (raw);
	}
}


public class MultipassToken
{
	[JsonProperty ("unique_id")]
	public Guid Id { get; set; }
	
	[JsonProperty ("name")]
	public string Name { get; set; }
	
	[JsonProperty ("email")]
	public string Email { get; set; }
	
	[JsonProperty ("expires")]
	public DateTime TokenExpires { get; set; }
}
