class Utils {
  static String formatBytes(int bytes) {
    if (bytes >= 1024 * 1024 * 1024) {
      return '${(bytes / (1024 * 1024 * 1024)).toStringAsFixed(2)} GB';
    }
    if (bytes >= 1024 * 1024) {
      return '${(bytes / (1024 * 1024)).toStringAsFixed(2)} MB';
    }
    if (bytes >= 1024) {
      return '${(bytes / 1024).toStringAsFixed(2)} KB';
    }
    return '$bytes bytes';
  }

  static String formatUptime(int seconds) {
    final h = seconds ~/ 3600;
    final m = (seconds % 3600) ~/ 60;
    final s = seconds % 60;
    if (h > 0) {
      return '$h h $m min $s s';
    }
    if (m > 0) {
      return '$m min $s s';
    }
    return '$s s';
  }

  static String formatAuthStatus(bool isOwner) => isOwner ? 'Proprietario' : 'Guest';

  static Color getConnectionStatusColor(dynamic state) {
    if (state is ConnectionState) {
      switch (state) {
        case ConnectionState.connected:
          return Colors.green;
        case ConnectionState.connecting:
          return Colors.orange;
        case ConnectionState.disconnected:
          return Colors.red;
        case ConnectionState.timeout:
          return Colors.orange;
        case ConnectionState.apiError:
          return Colors.red;
        case ConnectionState.firmwareIncompatible:
          return Colors.red;
        case ConnectionState.networkError:
          return Colors.red;
        default:
          return Colors.grey;
      }
    }
    return Colors.grey;
  }
}