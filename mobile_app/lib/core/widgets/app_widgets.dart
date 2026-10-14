class AppBarWidget extends StatelessWidget implements PreferredSizeWidget {
  final String title;
  final VoidCallback? onBack;
  final List<Widget>? actions;

  const AppBarWidget({
    super.key,
    required this.title,
    this.onBack,
    this.actions,
  });

  @override
  Size get preferredSize => const Size.fromHeight(kToolbarHeight);

  @override
  Widget build(BuildContext context) => AppBar(
    title: Text(title),
    leading: onBack != null
        ? IconButton(
            icon: const Icon(Icons.arrow_back),
            onPressed: onBack,
          )
        : null,
    actions: actions,
  );
}

class LoadingIndicator extends StatelessWidget {
  const LoadingIndicator({super.key});

  @override
  Widget build(BuildContext context) => const Center(
    child: CircularProgressIndicator(
      color: Color(0xFF00D4AA),
    ),
  );
}

ErrorWidgetBuilder? errorWidgetBuilder(BuildContext context, Object error, StackTrace stackTrace) {
  return (context, _, __) => Center(
    child: Text(
      'Errore: $error',
      style: const TextStyle(color: Colors.white),
    ),
  );
}