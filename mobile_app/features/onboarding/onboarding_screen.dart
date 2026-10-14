class OnboardingScreen extends StatefulWidget {
  const OnboardingScreen({super.key});

  @override
  State<OnboardingScreen> createState() => _OnboardingScreenState();
}

class _OnboardingScreenState extends State<OnboardingScreen>
    with SingleTickerProviderStateMixin {
  late final AnimationController _animController;
  late final Animation<double> _fadeAnimation;
  late final Animation<Offset> _slideAnimation;
  late final PageController _pageController;
  int _currentStep = 0;

  @override
  void initState() {
    super.initState();
    _animController = AnimationController(vsync: this, duration: const Duration(milliseconds: 800));
    _fadeAnimation = Tween<double>(begin: 0, end: 1).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOut));
    _slideAnimation = Tween<Offset>(begin: const Offset(0, 0.3), end: Offset.zero).animate(CurvedAnimation(parent: _animController, curve: Curves.easeOutCubic));
    _pageController = PageController();
    _animController.forward();
  }

  @override
  void dispose() {
    _animController.dispose();
    _pageController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) => Scaffold(
    body: FadeSlideTransition(
      fadeAnimation: _fadeAnimation,
      slideAnimation: _slideAnimation,
      child: _buildOnboardingBody(),
    ),
    bottomNavigationBar: _buildBottomBar(context),
  );

  Widget _buildOnboardingBody() => AnimatedBuilder(
    animation: _pageController,
    builder: (context, child) => PageView(
      controller: _pageController,
      physics: const NeverScrollableScrollPhysics(),
      children: [
        _buildStep1(),
        _buildStep2(),
        _buildStep3(),
      ],
    ),
  );

  Widget _buildStep1 => _buildStep(
    'Benvenuto in PortaChiave',
    'L\'app ti guiderà nella configurazione del dispositivo.',
    '1. Accendi PortaChiave',
    Icons.devices,
  );

  Widget _buildStep2 => _buildStep(
    'Connessione Wi-Fi',
    'Collega il telefono alla rete del dispositivo.',
    '2. Apri le impostazioni Wi-Fi\nseleziona PortaChiave-XXXX',
    Icons.wifi,
  );

  Widget _buildStep3 => _buildStep(
    'Completamento',
    'Accedi all\'app e inizia a controllare',
    '3. Torna nell\'app\n4. Premi "Connetti a PortaChiave"',
    Icons.check_circle,
  );

  Widget _buildStep(String title, String subtitle, String steps, IconData icon) => FadeSlideTransition(
    fadeAnimation: _fadeAnimation,
    slideAnimation: _slideAnimation,
    child: Padding(
      padding: const EdgeInsets.all(24.0),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Icon(icon, size: 48, color: const Color(0xFF00D4AA)),
          const SizedBox(height: 24),
          Text(
            title,
            style: const TextStyle(fontSize: 28, fontWeight: FontWeight.bold, color: Colors.white),
          ),
          const SizedBox(height: 8),
          Text(
            subtitle,
            style: const TextStyle(color: Colors.white70, fontSize: 16),
          ),
          const Spacer(),
          NavigationButton(
            onTap: _nextStep,
            text: _currentStep < 2 ? 'Avanti' : 'Inizia',
            icon: Icons.arrow_forward,
          ),
        ],
      ),
    ),
  );

  Widget _buildBottomBar(BuildContext context) => Container(
    color: const Color(0xFF1A1A24),
    padding: const EdgeInsets.all(24.0),
    child: Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        if (_currentStep > 0)
          TextButton(
            onPressed: _prevStep,
            child: const Text('Indietro'),
          ),
        const Spacer(),
        ElevatedButton(
          onPressed: _nextStep,
          child: _currentStep < 2 ? const Text('Avanti') : const Text('Inizia'),
        ),
      ],
    ),
  );

  void _nextStep() {
    if (_currentStep < 2) {
      setState(() => _currentStep++);
      _pageController.nextPage(
        duration: const Duration(milliseconds: 300),
        curve: Curves.easeOutCubic,
      );
    } else {
      context.go('/connection');
    }
  }

  void _prevStep() {
    if (_currentStep > 0) {
      setState(() => _currentStep--);
      _pageController.previousPage(
        duration: const Duration(milliseconds: 300),
        curve: Curves.easeOut,
      );
    }
  }
}

class NavigationButton extends StatelessWidget {
  final VoidCallback onTap;
  final String text;
  final IconData icon;

  const NavigationButton({
    required this.onTap,
    required this.text,
    required this.icon,
    super.key,
  });

  @override
  Widget build(BuildContext context) => GestureDetector(
    onTap: onTap,
    child: Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Icon(icon, color: Colors.white),
        const SizedBox(width: 8),
        Text(text, style: const TextStyle(color: Colors.white)),
      ],
    ),
  );
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