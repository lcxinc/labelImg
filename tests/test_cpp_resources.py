from pathlib import Path
import re
import unittest


class CppResourceTests(unittest.TestCase):
    def test_every_cpp_string_lookup_exists_in_all_bundles(self) -> None:
        root = Path(__file__).resolve().parents[1]
        source = "\n".join(
            path.read_text(encoding="utf-8")
            for path in (root / "cpp" / "src").rglob("*.cpp")
        )
        keys = {
            match.group(1) or match.group(2)
            for match in re.finditer(
                r"\.get\((?:QStringLiteral\(\"([^\"]+)\"\)|\"([^\"]+)\")\)",
                source,
            )
        }
        self.assertTrue(keys)

        for bundle in sorted((root / "resources" / "strings").glob("strings*.properties")):
            lines = [
                line
                for line in bundle.read_text(encoding="utf-8").splitlines()
                if line.strip()
                and not line.lstrip().startswith(("#", ";"))
                and "=" in line
            ]
            parsed_keys = [line.split("=", 1)[0].strip() for line in lines]
            bundle_keys = set(parsed_keys)
            duplicate_keys = {
                key for key in parsed_keys if parsed_keys.count(key) > 1
            }
            self.assertEqual(set(), duplicate_keys, f"duplicate keys in {bundle.name}")
            self.assertEqual(
                set(),
                keys - bundle_keys,
                f"missing C++ strings in {bundle.name}",
            )


if __name__ == "__main__":
    unittest.main()
