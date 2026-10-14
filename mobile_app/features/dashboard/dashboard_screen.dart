class DashboardScreen extends StatefulWidget {
  const DashboardScreen({super.key});

  @override
  State<DashboardScreen> createState() => _DashboardScreenState();
}

class _DashboardScreenState extends State<DashboardScreen>
    with SingleTickerProviderStateMixin, WidgetsBindingObserver {
  late final ApiClient _apiClient;
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  late final TabController _tabController;
  Map<String, dynamic>? _deviceData;
  Map<String, dynamic>? _systemData;
  bool _isAuthenticated = false;
  bool _isLoading = true;
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 1000));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _animController.forward();
    WidgetsBinding.instance.addObserver(this);
    _loadData();
  }

  @override
  void dispose() {
    _animController.dispose();
    WidgetsBinding.instance.removeObserver(this);
    super.dispose();
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (state == AppLifecycleState.resumed) {
      _refreshData();
    }
  }

  Future<void> _loadData() async {
    _apiClient = ApiClient();
    try {
      final deviceData = await _apiClient.get('/api/device');
      final systemData = await _apiClient.get('/api/system');
      final authData = await _apiClient.get('/api/auth/status');
      setState(() {
        _deviceData = deviceData;
        _systemData = systemData;
        _isAuthenticated = authData['isOwner'] ?? false;
        _isLoading = false;
      });
    } on ApiException catch (e) {
      _connectionState.setState(ConnectionState.apiError, e.message);
      setState(() => _isLoading = false);
    }
  }

  Future<void> _refreshData() async {
    try {
      final deviceData = await _apiClient.get('/api/device');
      final systemData = await _apiClient.get('/api/system');
      setState(() {
        _deviceData = deviceData;
        _systemData = systemData;
      });
    } catch (_) {
      // Silently handle refresh failures
    }
  }

  void _login() {
    context.go('/auth');
  }

  void _logout() {
    setState(() => _isAuthenticated = false);
    context.go('/auth');
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

  Widget _buildBody() => CustomScrollView(
    slivers: [
      _buildAppBar(),
      const SliverToBoxAdapter(child: _buildConnectionStatusBar()),
      ExpandableTabBarView(
        controller: _tabController,
        children: [
          _buildConnectionTab(),
          _buildSystemTab(),
          _buildAuthTab(),
        ],
      ),
    ],
  );

  Widget _buildAppBar() => SliverAppBar(
    expandedHeight: kToolbarHeight + 60,
    floating: true,
    pinned: true,
    backgroundColor: const Color(0xFF111118),
    foregroundColor: Colors.white,
    title: const Text('PortaChiave'),
    actions: [
      IconButton(
        icon: Icon(_isAuthenticated ? Icons.logout : Icons.login),
        onPressed: _isAuthenticated ? _logout : _login,
      ),
    ],
    bottom: _buildBottomTabBar(),
  );

  Widget _buildConnectionStatusBar() => Card(
    color: Utils.getConnectionStatusColor(_connectionState.state).withOpacity(0.1),
    child: const Padding(
      padding: EdgeInsets.all(8.0),
      child: Row(
        children: [
          Icon(Icons.wifi, color: Colorsgreen, size: 20),
          SizedBox(width: 8),
          Text('Stato connessione:', style: TextStyle(color: Colors.white)),
        ],
      ),
    ),
  );

  Widget _buildConnectionTab => _buildDeviceInfoCard();
  Widget _buildSystemTab => _buildSystemInfoCard();
  Widget _buildAuthTab => _buildAuthInfoCard();

  Widget _buildDeviceInfoCard => Card(
    child: Padding(
      padding: const EdgeInsets.all(20.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Informazioni Dispositivo', style: TextStyle(fontSize: 18, color: Colors.white)),
          const SizedBox(height: 16),
          _infoRow('Nome', _deviceData?['name'] ?? 'PortaChiave'),
          _infoRow('ID', _deviceData?['id'] ?? 'N/A'),
          _infoRow('Firmware', _deviceData?['firmware'] ?? '1.0.0'),
          _infoRow('Memoria libera', Utils.formatBytes(_deviceData?['freeMemory'] ?? 0)),
          _infoRow('Uptime', Utils.formatUptime(_deviceData?['uptime'] ?? 0)),
          _infoRow('Autenticazione', Utils.formatAuthStatus(_isAuthenticated)),
          const SizedBox(height: 24),
          SizedBox(
            width: double.infinity,
            child: ElevatedButton(
              onPressed: _refreshData,
              child: const Text('Aggiorna'),
            ),
          ),
        ],
      ),
    ),
  );

  Widget _buildSystemInfoCard => Card(
    child: Padding(
      padding: const EdgeInsets.all(20.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Informazioni Sistema', style: TextStyle(fontSize: 18, color: Colors.white)),
          const SizedBox(height: 16),
          _infoRow('FPS Robot', _systemData?['fps'] ?? 'N/A'),
          _infoRow('Memoria totale', Utils.formatBytes(_systemData?['totalMemory'] ?? 0)),
          _infoRow('Sketch libero', Utils.formatBytes(_systemData?['sketchFree'] ?? 0)),
          _infoRow('Motivo reset', _systemData?['resetReason'] ?? 'N/A'),
          _infoRow('Uptime sistema', Utils.formatUptime(_systemData?['uptime'] ?? 0)),
          const SizedBox(height: 24),
          const Text('Aggiornamento in corso...', style: TextStyle(color: Colors.white54)),
        ],
      ),
    ),
  );

  Widget _buildAuthInfoCard => Card(
    child: Padding(
      padding: const EdgeInsets.all(20.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Autenticazione', style: TextStyle(fontSize: 18, color: Colors.white)),
          const SizedBox(height: 16),
          _infoRow('Stato', _isAuthenticated ? 'Proprietario' : 'Guest'),
          _infoRow('Ruolo', _isAuthenticated ? 'Owner' : 'Guest'),
          const SizedBox(height: 24),
          ElevatedButton.icon(
            onPressed: _isAuthenticated ? _logout : _login,
            icon: Icon(_isAuthenticated ? Icons.logout : Icons.login),
            label: Text(_isAuthenticated ? 'Logout' : 'Login'),
          ),
        ],
      ),
    ),
  );

  Widget _buildBottomTabBar() => Container(
    color: const Color(0xFF1A1A24),
    child: TabBar(
      controller: _tabController,
      tabs: const [
        Tab(icon: Icon(Icons.devices), text: 'Dispositivo'),
        Tab(icon: Icon(Icons.settings_system), text: 'Sistema'),
        Tab(icon: Icon(Icons.security), text: 'Autenticazione'),
      ],
    ),
  );

  Widget _infoRow(String label, dynamic value) {
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(color: Colors.white54)),
          Text(value.toString(), style: const TextStyle(color: Colors.white)),
        ],
      ),
    );
  }
}

class ExpandableTabBarView extends StatelessWidget {
  final TabController controller;
  final List<Widget> children;

  const ExpandableTabBarView({
    required this.controller,
    required this.children,
    super.key,
  });

  @override
  Widget build(BuildContext context) => TabBarView(
    controller: controller,
    children: children,
  );
}