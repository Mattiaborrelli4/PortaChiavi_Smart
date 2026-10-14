class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen>
    with SingleTickerProviderStateMixin {
  final ApiClient _apiClient = ApiClient();
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  String _deviceName = 'PortaChiave';
  String _selectedAuthMode = 'Owner';
  bool _sessionTimeout = true;
  int _timeoutMinutes = 30;
  bool _isLoading = true;
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;

  static const List<String> _authModes = ['Owner', 'Guest', 'Limited'];

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 600));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _animController.forward();
    _loadSettings();
  }

  @override
  void dispose() {
    _animController.dispose();
    super.dispose();
  }

  Future<void> _loadSettings() async {
    try {
      final data = await _apiClient.get('/api/device');
      setState(() {
        _deviceName = data['name'] ?? 'PortaChiave';
        _selectedAuthMode = data['authMode'] ?? 'Owner';
        _sessionTimeout = data['sessionTimeout'] ?? true;
        _timeoutMinutes = data['timeoutMinutes'] ?? 30;
        _isLoading = false;
      });
    } catch (_) {
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
          'Impostazioni',
          style: TextStyle(fontSize: 32, fontWeight: FontWeight.bold, color: Colors.white),
        ),
        const SizedBox(height: 8),
        const Text(
          'Configura il comportamento del dispositivo',
          style: TextStyle(color: Colors.white54, fontSize: 14),
        ),
        const Divider(color: Color(0xFF333366), height: 32),
        const SizedBox(height: 24),
        _buildSettingsTile(
          title: 'Nome dispositivo',
          subtitle: _deviceName,
          leading: Icons.devices,
          onTap: _showDeviceNameDialog,
        ),
        _buildSettingsTile(
          title: 'Modalità autenticazione',
          subtitle: _selectedAuthMode,
          leading: Icons.security,
          onTap: _showAuthModeDialog,
        ),
        _buildSettingsTile(
          title: 'Timeout sessione',
          subtitle: '${_timeoutMinutes} minuti',
          leading: Icons.timer,
          trailing: Switch(
            value: _sessionTimeout,
            activeColor: Color(0xFF00D4AA),
            onChanged: (value) => setState(() => _sessionTimeout = value),
          ),
          onTap: _showTimeoutDialog,
        ),
        const SizedBox(height: 32),
        _buildResetButton(),
      ],
    ),
  );

  Widget _buildSettingsTile({
    required String title,
    required String subtitle,
    required IconData leading,
    required VoidCallback onTap,
    Widget? trailing,
    Widget? titleStyle,
  }) => Card(
    child: ListTile(
      leading: Icon(leading, color: const Color(0xFF00D4AA)),
      title: Text(
        title,
        style: const TextStyle(color: Colors.white),
      ),
      subtitle: Text(
        subtitle,
        style: const TextStyle(color: Colors.white54),
      ),
      trailing: trailing,
      onTap: onTap,
    ),
  );

  void _showDeviceNameDialog() {
    final controller = TextEditingController(text: _deviceName);
    showDialog(
      context: context,
      builder: => AlertDialog(
        backgroundColor: const Color(0xFF1A1A24),
        title: const Text('Nome dispositivo', style: TextStyle(color: Colors.white)),
        content: TextField(
          controller: controller,
          style: const TextStyle(color: Colors.white),
          decoration: const InputDecoration(
            labelText: 'Inserisci nome',
            labelStyle: TextStyle(color: Colors.white54),
            filled: true,
            fillColor: Color(0xFF1A1A24),
            border: OutlineInputBorder(
              borderRadius: BorderRadius.circular(12),
              borderSide: BorderSide.none,
            ),
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Annulla', style: TextStyle(color: Colors.white54)),
          ),
          ElevatedButton(
            onPressed: () {
              setState(() => _deviceName = controller.text);
              Navigator.of(context).pop();
            },
            style: ElevatedButton.styleFrom(backgroundColor: Color(0xFF00D4AA)),
            child: const Text('Salva'),
          ),
        ],
      ),
    );
  }

  void _showAuthModeDialog() {
    showDialog(
      context: context,
      builder: => AlertDialog(
        backgroundColor: const Color(0xFF1A1A24),
        title: const Text('Modalità autenticazione', style: TextStyle(color: Colors.white)),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: _authModes.map((mode) => ListTile(
            title: Text(mode,
                style: TextStyle(color: _selectedAuthMode == mode ? Colors.green : Colors.white)),
            selected: _selectedAuthMode == mode,
            onTap: () {
              setState(() => _selectedAuthMode = mode);
              Navigator.of(context).pop();
            },
          )).toList(),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Annulla', style: TextStyle(color: Colors.white54)),
          ),
        ],
      ),
    );
  }

  void _showTimeoutDialog() {
    final controller = TextEditingController(text: _timeoutMinutes.toString());
    showDialog(
      context: context,
      builder: => AlertDialog(
        backgroundColor: const Color(0xFF1A1A24),
        title: const Text('Timeout sessione (minuti)', style: TextStyle(color: Colors.white)),
        content: TextField(
          controller: controller,
          keyboardType: TextInputType.number,
          style: const TextStyle(color: Colors.white),
          decoration: const InputDecoration(
            labelText: 'Minuti',
            labelStyle: TextStyle(color: Colors.white54),
            filled: true,
            fillColor: Color(0xFF1A1A24),
            border: OutlineInputBorder(
              borderRadius: BorderRadius.circular(12),
              borderSide: BorderSide.none,
            ),
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('Annulla', style: TextStyle(color: Colors.white54)),
          ),
          ElevatedButton(
            onPressed: () {
              final minutes = int.tryParse(controller.text) ?? 30;
              setState(() => _timeoutMinutes = minutes);
              Navigator.of(context).pop();
            },
            style: ElevatedButton.styleFrom(backgroundColor: Color(0xFF00D4AA)),
            child: const Text('Salva'),
          ),
        ],
      ),
    );
  }

  Widget _buildResetButton() => Card(
    color: Colors.red[900].withOpacity(0.1),
    child: Padding(
      padding: const EdgeInsets.all(16.0),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          const Text('Ripristina impostazioni', style: TextStyle(color: Colors.redAccent)),
          const SizedBox(width: 8),
          Icon(Icons.restart_alt, color: Colors.redAccent),
        ],
      ),
    ),
  );
}