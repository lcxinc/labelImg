from pathlib import Path
import unittest


class PackagingTests(unittest.TestCase):
    def test_windows_package_copies_ai_bridge_next_to_executable(self) -> None:
        script = Path(__file__).resolve().parents[1] / "cpp" / "packaging" / "build_windows_installer.ps1"
        text = script.read_text(encoding="utf-8")
        self.assertIn("labelme_ai_bridge.py", text)
        self.assertRegex(text, r"Copy-Item[\s\S]*labelme_ai_bridge\.py[\s\S]*deployRoot")
        self.assertIn("exit 0", text)
        self.assertIn("Start-Process", text)
        self.assertIn("Stop-Process", text)
        self.assertIn("while (!$iexpress.HasExited", text)
        self.assertIn("$installerSize", text)
        self.assertIn("$payloadSize", text)


if __name__ == "__main__":
    unittest.main()
