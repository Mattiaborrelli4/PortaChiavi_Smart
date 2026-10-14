class AuthScreen extends StatefulWidget {
  const AuthScreen({super.key});

  @override
  State<AuthScreen> createState() => _AuthScreenState();
}

class _AuthScreenState extends State<AuthScreen> with SingleTickerProviderStateMixin {
  final ApiClient _apiClient = ApiClient();
  final ConnectionStateModel _connectionState = ConnectionStateModel();
  final _formKey = GlobalKey<FormState>();
  final _emailController = TextEditingController();
  final _passwordController = TextEditingController();
  bool _isLoading = false;
  bool _isOwner = false;
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;
  late final Animation<Offset> _slideAnimation;

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 800));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _slideAnimation = Tween<Offset>(begin: const Offset(0, 0.5), end: Offset.zero).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOutCubic));
    _animController.forward();
  }

  @override
  void dispose() {
    _animController.dispose();
    _emailController.dispose();
    _passwordController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Scaffold(
    body: FadeSlideTransition(
      fadeAnimation: _fadeAnimation,
      slideAnimation: _slideAnimation,
      child: AuthBody(
        apiClient: _apiClient,
        connectionState: _connectionState,
        onLogin: _login,
        onContinueAsGuest: _continueAsGuest,
      ),
    ),
  );

  Future<void> _login() async {
    if (!_formKey.currentState!.validate()) return;
    setState(() => _isLoading = true);
    try {
      final data = await _apiClient.get('/api/auth/login');
      setState(() => _isLoading = false);
      if (!mounted) return;
      setState(() => _isOwner = data['isOwner'] ?? false);
      context.go('/dashboard');
    } on ApiException catch (e) {
      setState(() => _isLoading = false);
      _showError('Errore login: ${e.message}');
    }
  }

  Future<void> _continueAsGuest() async {
    try {
      final data = await _apiClient.get('/api/auth/status');
      setState(() => _isOwner = data['isOwner'] ?? false);
      context.go('/dashboard');
    } on ApiException catch (e) {
      _showError('Errore accesso guest: ${e.message}');
    }
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
}

class FadeSlideTransition extends StatelessWidget {
  final Animation<double> fadeAnimation;
  final Animation<Offset> slideAnimation;
  final Widget child;

  const FadeSlideTransition({
    required this.fadeAnimation,
    required this.slideAnimation,
    required this.child,
    super.key,
  });

  @override
  Widget build(BuildContext context) => SlideTransition(
    position: slideAnimation,
    child: FadeTransition(
      opacity: fadeAnimation,
      child: child,
    ),
  );
}