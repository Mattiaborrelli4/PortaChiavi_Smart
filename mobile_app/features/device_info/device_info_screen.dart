class DeviceInfoScreen extends StatefulWidget {
  const DeviceInfoScreen({super.key});

  @override
  State<DeviceInfoScreen> createState() => _DeviceInfoScreenState();
}

class _DeviceInfoScreenState extends State<DeviceInfoScreen>
    with SingleTickerProviderStateMixin {
  final ApiClient _apiClient = ApiClient();
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  Map<String, dynamic>? _deviceData;
  bool _isLoading = true;
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 600));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _animController.forward();
    _loadData();
  }

  @override
  void dispose() {
    _animController.dispose();
    super.dispose();
  }

  Future<void> _loadData() async {
    try {
      final data = await _apiClient.get('/api/device');
      setState(() => _deviceData = data);
      setState(() => _isLoading = false);
    } on ApiException catch (e) {
      _connectionState.setState(ConnectionState.apiError, e.message);
      setState(() => _isLoading = false);
    }
  }

  @override
  Widget build(BuildContext context) => Scaffold(
    body: FadeSlideTransition(
      fadeAnimation: _fadeAnimation,
      child: _isLoading
          ? const Center(child: LoadingIndicator())
          : _buildBody(),
    ),
  );

  Widget _buildBody() => SingleChildScrollView(
    padding: const EdgeInsets.all(24.0),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'Informazioni Dispositivo',
          style: TextStyle(fontSize: 32, fontWeight: FontWeight.bold, color: Colors.white),
        ),
        const SizedBox(height: 8),
        const Text(
          'Dettagli del dispositivo PortaChiave',
          style: TextStyle(color: Colors.white54, fontSize: 14),
        ),
        const Divider(color: Color(0xFF333366), height: 32),
        const SizedBox(height: 24),
        _buildInfoCard('Nome', _deviceData?['name'] ?? 'PortaChiave',
            _deviceData?['name'] != null ? Icons.devices : null),
        _buildInfoCard('ID', _deviceData?['id'] ?? 'N/A', Icons.id),
        _buildInfoCard('Firmware', _deviceData?['firmware'] ?? '1.0.0', Icons.update),
        _buildInfoCard('Memoria libera',
            Utils.formatBytes(_deviceData?['freeMemory'] ?? 0),
            Icons.storage),
        _buildInfoCard('Uptime', Utils.formatUptime(_deviceData?['uptime'] ?? 0),
            Icons.access_time),
        _buildInfoCard('Autenticazione',
            Utils.formatAuthStatus(_deviceData?['isAuthenticated'] ?? false),
            Icons.security),
        const SizedBox(height: 32),
        SizedBox(
          width: double.infinity,
          child: ElevatedButton(
            onPressed: _refreshData,
            child: const Text('Aggiorna dati'),
          ),
        ),
      ],
    ),
  );

  Widget _buildInfoCard(String label, dynamic value, IconData? icon) => Card(
    child: Padding(
      padding: const EdgeInsets.all(16.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Row(
            children: [
              Icon(icon, color: const Color(0xFF00D4AA), size: 20),
              const SizedBox(width: 8),
              Text(
                label,
                style: const TextStyle(color: Colors.white54, fontSize: 16),
              ),
            ],
          ),
          Text(
            value.toString(),
            style: const TextStyle(color: Colors.white, fontSize: 16),
          ),
        ],
      ),
    ),
  );

  Future<void> _refreshData() async {
    try {
      final data = await _apiClient.get('/api/device');
      setState(() => _deviceData = data);
      _showSuccess('Dati aggiornati');
    } catch (_) {
      _showError('Errore aggiornamento');
    }
  }

  void _showSuccess(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message), backgroundColor: Colors.green[700]),
    );
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(content: Text(message), backgroundColor: Colors.red[700]),
    );
  }
}