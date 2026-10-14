class ConnectionScreen extends StatefulWidget {
  const ConnectionScreen({super.key});

  @override
  State<ConnectionScreen> createState() => _ConnectionScreenState();
}

class _ConnectionScreenState extends State<ConnectionScreen>
    with SingleTickerProviderStateMixin {
  final ApiClient _apiClient = ApiClient();
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  final _formKey = GlobalKey<FormState>();
  final _ipController = TextEditingController(text: '192.168.4.1');
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;
  late final Animation<Offset> _slideAnimation;

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 600));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _slideAnimation = Tween<Offset>(begin: const Offset(0, 0.3), end: Offset.zero).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOutCubic));
    _animController.forward();
  }

  @override
  void dispose() {
    _animController.dispose();
    _ipController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Scaffold(
    body: FadeSlideTransition(
      fadeAnimation: _fadeAnimation,
      slideAnimation: _slideAnimation,
      child: ConnectionBody(
        apiClient: _apiClient,
        connectionState: _connectionState,
        ipController: _ipController,
        onConnect: _connect,
      ),
    ),
  );

  Future<void> _connect() async {
    if (!_formKey.currentState!.validate()) return;
    _formKey.currentState!.save();
    _connectionState.setState(ConnectionState.connecting);
    try {
      final String ip = _ipController.text.trim();
      final baseUrl = ip.isNotEmpty ? 'http://$ip' : DeviceConfig.baseUrl;
      final client = ApiClient(baseUrl: baseUrl);
      final data = await client.get('/api/device');
      if (!mounted) return;
      context.go('/dashboard');
    } on ApiException catch (e) {
      _connectionState.setState(ConnectionState.apiError, e.message);
      if (!mounted) return;
      _showError('Errore connessione: ${e.message}');
    } catch (_) {
      _connectionState.setState(ConnectionState.networkError);
      if (!mounted) return;
      _showError('Errore di rete');
    }
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            const Icon(Icons.warning_amber_outlined, color: Colors.white),
            const SizedBox(width: 8),
            Expanded(child: Text(message)),
          ],
        ),
        backgroundColor: Colors.orange[700],
        behavior: SnackBar.floating,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        margin: const EdgeInsets.all(16),
      ),
    );
  }
}

class ConnectionBody extends StatelessWidget {
  final ApiClient apiClient;
  final ConnectionStateModel connectionState;
  final TextEditingController ipController;
  final VoidCallback onConnect;

  const ConnectionBody({
    required this.apiClient,
    required this.connectionState,
    required this.ipController,
    required this.onConnect,
    super.key,
  });

  @override
  Widget build(BuildContext context) => SingleChildScrollView(
    padding: const EdgeInsets.all(24.0),
    child: Form(
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text(
            'PortaChiave',
            style: TextStyle(fontSize: 36, fontWeight: FontWeight.bold, color: Colors.white),
          ),
          const SizedBox(height: 8),
          const Text(
            'Connessione al dispositivo',
            style: TextStyle(color: Colors.white54, fontSize: 16),
          ),
          const SizedBox(height: 32),
          TextFormField(
            controller: ipController,
            decoration: InputDecoration(
              labelText: 'Indirizzo IP',
              labelStyle: const TextStyle(color: Colors.white54),
              hintText: '192.168.4.1',
              hintStyle: const TextStyle(color: Colors.white54),
              filled: true,
              fillColor: const Color(0xFF1A1A24),
              border: OutlineInputBorder(
                borderRadius: BorderRadius.circular(12),
                borderSide: BorderSide.none,
              ),
              enabledBorder: OutlineInputBorder(
                borderRadius: BorderRadius.circular(12),
                borderSide: const BorderSide(color: Color(0xFF333366)),
              ),
              focusedBorder: OutlineInputBorder(
                borderRadius: BorderRadius.circular(12),
                borderSide: const BorderSide(color: Color(0xFF00D4AA)),
              ),
            ),
            style: const TextStyle(color: Colors.white),
            validator: (value) => value?.isEmpty ?? true ? 'Inserisci un indirizzo IP' : null,
          ),
          const SizedBox(height: 24),
          _buildConnectionStatus(context),
          const SizedBox(height: 32),
          ElevatedButton(
            onPressed: () => onConnect(),
            child: _connectionState.state == ConnectionState.connecting
                ? const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator(
                      strokeWidth: 2,
                      color: Colors.white,
                    ),
                  )
                : const Text('Connetti'),
            style: ElevatedButton.styleFrom(
              backgroundColor: const Color(0xFF00D4AA),
              foregroundColor: Colors.black,
              elevation: 0,
              minimumSize: const Size(double.infinity, 50),
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
            ),
          ),
          const SizedBox(height: 16),
          const Text(
            'Attenzione: La rete del dispositivo potrebbe non avere accesso a Internet',
            style: TextStyle(color: Colors.white54, fontSize: 12, fontStyle: Italic),
            textAlign: TextAlign.center,
          ),
        ],
      ),
    ),
  );

  Widget _buildConnectionStatus(BuildContext context) => Card(
    color: Utils.getConnectionStatusColor(_connectionState.state).withOpacity(0.2),
    child: Padding(
      padding: const EdgeInsets.all(12.0),
      child: Row(
        children: [
          Icon(
            _connectionIcon,
            color: Utils.getConnectionStatusColor(_connectionState.state),
            size: 20,
          ),
          const SizedBox(width: 8),
          Expanded(
            child: Text(
              _connectionState.state.toString().split('.').last,
              style: const TextStyle(color: Colors.white),
              overflow: TextOverflow.ellipsis,
            ),
          ),
        ],
      ),
    ),
  );

  IconData get _connectionIcon {
    switch (_connectionState.state) {
      case ConnectionState.connected:
        return Icons.wifi;
      case ConnectionState.connecting:
        return Icons.wifi_searching;
      case ConnectionState.disconnected:
        return Icons.wifi_off;
      default:
        return Icons.question_mark;
    }
  }
}