class RobotEyesScreen extends StatefulWidget {
  const RobotEyesScreen({super.key});

  @override
  State<RobotEyesScreen> createState() => _RobotEyesScreenState();
}

class _RobotEyesScreenState extends State<RobotEyesScreen>
    with SingleTickerProviderStateMixin {
  final ApiClient _apiClient = ApiClient();
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  String _emotion = 'neutro';
  bool _sleepMode = false;
  bool _animationsEnabled = true;
  int _selectedAnimation = 0;
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;
  late final Animation<Offset> _slideAnimation;

  static const List<String> _emotions = ['neutro', 'felice', 'triste', 'arrabbiato', 'sorpreso'];
  static const List<String> _animations = ['Blink', 'Doppio blink', 'Wink', 'Sleepy', 'Cuore'];

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 500));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _slideAnimation = Tween<Offset>(begin: const Offset(0, 0.3), end: Offset.zero).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOutCubic));
    _animController.forward();
    _loadEmotion();
  }

  @override
  void dispose() {
    _animController.dispose();
    super.dispose();
  }

  Future<void> _loadEmotion() async {
    try {
      final data = await _apiClient.get('/api/robot/emotion');
      setState(() {
        _emotion = data['currentEmotion'] ?? 'neutro';
        _sleepMode = data['sleepMode'] ?? false;
        _animationsEnabled = data['animationsEnabled'] ?? true;
      });
    } catch (_) {
      // Silley handle
    }
  }

  Future<void> _setEmotion() async {
    try {
      final result = await _apiClient.post('/api/robot/emotion', {
        'emotion': _emotion,
        'animationsEnabled': _animationsEnabled,
      });
      if (!mounted) return;
      _showSuccess('Emozione impostata: $_emotion');
    } on ApiException catch (e) {
      _showError('Errore impostazione emozione: ${e.message}');
    }
  }

  Future<void> _setSleepMode() async {
    try {
      setState(() => _sleepMode = !_sleepMode);
      final result = await _apiClient.post('/api/robot/sleep', {
        'sleepMode': _sleepMode,
      });
      if (!mounted) {
        setState(() => _sleepMode = !_sleepMode);
        return;
      }
      _showSuccess('Sleep mode: ${_sleepMode ? 'Attivato' : 'Disattivato'}');
    } catch (_) {
      setState(() => _sleepMode = !_sleepMode);
      _showError('Errore modalità sleep');
    }
  }

  Future<void> _setAnimation() async {
    try {
      final result = await _apiClient.post('/api/robot/animation', {
        'animationIndex': _selectedAnimation,
        'enabled': _animationsEnabled,
      });
      if (!mounted) return;
      _showSuccess('Animazione: ${_animations[_selectedAnimation]}');
    } on ApiException catch (e) {
      _showError('Errore animazione: ${e.message}');
    }
  }

  IconData _getEmotionIcon(String emotion) {
    switch (emotion) {
      case 'felice':
        return Icons.sentiment_satisfied;
      case 'triste':
        return Icons.sentiment_dissatisfied;
      case 'arrabbiato':
        return Icons.sentiment_very_dissatisfied;
      case 'sorpreso':
        return Icons.monetization_on;
      default:
        return Icons.sentiment_neutral;
    }
  }

  Color _getEmotionColor(String emotion) {
    switch (emotion) {
      case 'felice':
        return Colors.green;
      case 'triste':
        return Colors.blue;
      case 'arrabbiato':
        return Colors.red;
      case 'sorpreso':
        return Colors.purple;
      default:
        return Colors.grey;
    }
  }

  void _showSuccess(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            const Icon(Icons.check_circle_outline, color: Colors.white),
            const SizedBox(width: 8),
            Expanded(child: Text(message)),
          ],
        ),
        backgroundColor: Colors.green[700],
        behavior: SnackBar.floating,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        margin: const EdgeInsets.all(16),
      ),
    );
  }

  void _showError(String message) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Row(
          children: [
            const Icon(Icons.error_outline, color: Colors.white),
            const SizedBox(width: 8),
            Expanded(child: Text(message)),
          ],
        ),
        backgroundColor: Colors.red[700],
        behavior: SnackBar.floating,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        margin: const EdgeInsets.all(16),
      ),
    );
  }

  @override
  Widget build(BuildContext context) => Scaffold(
    body: FadeSlideTransition(
      fadeAnimation: _fadeAnimation,
      slideAnimation: _slideAnimation,
      child: _buildBody(),
    ),
  );

  Widget _buildBody() => SingleChildScrollView(
    padding: const EdgeInsets.all(24.0),
    child: Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text(
          'RobotEyes',
          style: TextStyle(fontSize: 32, fontWeight: FontWeight.bold, color: Colors.white),
        ),
        const SizedBox(height: 4),
        const Text(
          'Controllo occhi robotici',
          style: TextStyle(color: Colors.white54, fontSize: 14),
        ),
        const Divider(color: Color(0xFF333366), height: 32),
        _buildEmotionSection(),
        const SizedBox(height: 24),
        _buildSleepSection(),
        const SizedBox(height: 24),
        _buildAnimationSection(),
        const SizedBox(height: 32),
        _buildStatusCard(),
      ],
    ),
  );

  Widget _buildEmotionSection() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text('Emozione', style: TextStyle(fontSize: 18, color: Colors.white)),
            const SizedBox(height: 12),
            DropdownButtonFormField<String>(
              value: _emotion,
              decoration: InputDecoration(
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
                hintText: 'Seleziona emozione',
                hintStyle: const TextStyle(color: Colors.white54),
              ),
              style: const TextStyle(color: Colors.white),
              items: _emotions.map((String emotion) {
                return DropdownMenuItem<String>(
                  value: emotion,
                  child: Row(
                    children: [
                      Icon(_getEmotionIcon(emotion), color: _getEmotionColor(emotion), size: 20),
                      const SizedBox(width: 8),
                      Text(emotion, style: const TextStyle(fontSize: 16)),
                    ],
                  ),
                );
              }).toList(),
              onChanged: (value) {
                setState(() => _emotion = value ?? 'neutro');
              },
            ),
            const SizedBox(height: 16),
            Row(
              children: [
                const Expanded(child: Text('Animazioni:', style: TextStyle(color: Colors.white54))),
                const SizedBox(width: 8),
                Expanded(
                  child: DropdownButtonFormField<int>(
                    value: _selectedAnimation,
                    decoration: InputDecoration(
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
                      hintText: 'Animazione',
                      hintStyle: const TextStyle(color: Colors.white54),
                    ),
                    style: const TextStyle(color: Colors.white),
                    items: List.generate(_animations.length, (index) {
                      return DropdownMenuItem<int>(
                        value: index,
                        child: Text(_animations[index]),
                      );
                    }),
                    onChanged: (value) {
                      setState(() => _selectedAnimation = value ?? 0);
                    },
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildSleepSection() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: ListTile(
          leading: Icon(
            _sleepMode ? Icons.sleep : Icons.wake,
            color: Color(0xFF00D4AA),
          ),
          title: Text(
            _sleepMode ? 'Sleep mode: Attivato' : 'Sleep mode: Disattivato',
            style: TextStyle(color: _sleepMode ? Colors.green : Colors.white),
          ),
          subtitle: const Text('Disattiva gli occhi durante il sonno',
              style: TextStyle(color: Colors.white54)),
          trailing: Switch(
            value: _sleepMode,
            activeColor: Color(0xFF00D4AA),
            onChanged: (value) => _setSleepMode(),
          ),
        ),
      ),
    );
  }

  Widget _buildAnimationSection() {
    return Card(
      child: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text('Animazione corrente', style: TextStyle(fontSize: 18, color: Colors.white)),
            const SizedBox(height: 12),
            Row(
              children: [
                Expanded(
                  child: ElevatedButton(
                    onPressed: _setAnimation,
                    child: const Text('Imposta'),
                    style: ElevatedButton.styleFrom(
                      backgroundColor: const Color(0xFF00D4AA),
                      foregroundColor: Colors.black,
                      minimumSize: const Size(double.infinity, 40),
                    ),
                  ),
                ),
              ],
            ),
            const SizedBox(height: 12),
            DropdownButtonFormField<String>(
              value: _animationsEnabled ? 'enabled' : 'disabled',
              decoration: InputDecoration(
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
                hintText: 'Animazioni',
                hintStyle: const TextStyle(color: Colors.white54),
              ),
              style: const TextStyle(color: Colors.white),
              items: ['enabled', 'disabled'].map((String value) {
                return DropdownMenuItem<String>(
                  value: value,
                  child: Text(value),
                );
              }).toList(),
              onChanged: (value) {
                setState(() => _animationsEnabled = value == 'enabled');
              },
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStatusCard() => Card(
    color: Utils.getConnectionStatusColor(_connectionState.state).withOpacity(0.1),
    child: Padding(
      padding: const EdgeInsets.all(16.0),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          const Text('Stato connessione', style: TextStyle(fontSize: 16, color: Colors.white)),
          const SizedBox(height: 8),
          Row(
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