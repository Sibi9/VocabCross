# خطة التكامل والربط البرمجي (VocabCross - Arabic Dictionary Integration Plan)

تم اختيار واستنساخ نواة المشروع المفتوح المصدر: **LexiForge** (`game_core`)
المستودع الأصلي: [tanyaverma20/LexiForge](https://github.com/tanyaverma20/LexiForge)

---

## 1. تحديد مكان التحقق واكتشاف الكلمة المكتملة (Word Detection Location)

يتميز المشروع ببنية معمارية نظيفة (Clean Domain-Driven Architecture) تفصل منطق اللعبة (Domain) عن واجهات العرض (Presentation). يتم رصد الكلمة عند إتمام السحب في موقعين مترابطين:

### أ) على مستوى منطق اللعبة (Domain Layer):
* **الملف:** `game_core/lib/features/game/domain/game_controller.dart`
* **الدالة:** `SelectionResult select(Position start, Position end)` (الأسطر 39 - 111)
* **الشرح:**
  تقوم الدالة بتتبع خط السحب، واستخراج الحروف ومقارنتها بالكلمات المستهدفة:
  ```dart
  // السطر 71 - 77: مطابقة الكلمة
  String? matchedWord;
  for (final w in _state.puzzle.words.toSet()) {
    if (w == letters || w == reversed) {
      matchedWord = w;
      break;
    }
  }

  // السطر 105 - 110: إرجاع نتيجة الاكتشاف بنجاح
  return SelectionResult(
    status: SelectionStatus.found,
    cells: cells,
    word: matchedWord, // الكلمة التي تم حلها
    pointsAwarded: points,
  );
  ```

### ب) على مستوى واجهة المستخدم وتفاعل اللاعب (Presentation Layer):
* **الملف:** `game_core/lib/features/game/presentation/screens/game_screen.dart`
* **الدالة:** `void _onSelectionEnd(Position cell)` (الأسطر 193 - 215)
* **الشرح:**
  تُستدعى فور رفع المستخدم إصبعه عن الشاشة بعد سحب الحروف:
  ```dart
  void _onSelectionEnd(Position cell) {
    ...
    final result = _controller.select(start, cell);
    setState(() {
      _selectedCells = {};
      if (result.status == SelectionStatus.found) {
        // *** النقطة الجوهرية (Word Detection Hook) ***
        final String foundWord = result.word!;
        // هنا يتم استدعاء القاموس العربي / نافذة الترجمة
        _onWordDiscovered(foundWord);
        ...
      }
    });
  }
  ```

---

## 2. كيفية ربط القاموس العربي / خدمة الترجمة (Arabic Dictionary / Translation Integration)

بمجرد اكتشاف الكلمة المكتملة، يمكن تنفيذ أحد الخيارين التاليين (محلي أو عبر API):

### الخيار 1: قاعدة بيانات محلية سريعة (Local SQLite / Asset JSON) - [الموصى به لتجربة دون إنترنت]:
1. أضف ملف معجم بصيغة JSON داخل مجلد `assets/data/arabic_dictionary.json`:
   ```json
   {
     "REACT": {
       "translation": "يتفاعل / يبدي ردة فعل",
       "root": "فعل",
       "example": "He didn't react to the news."
     },
     "FLUTTER": {
       "translation": "يرفرف / يتحرك بسرعة",
       "root": "رفرف",
       "example": "The flag fluttered in the wind."
     }
   }
   ```
2. أنشئ خدمة استعلام `DictionaryService`:
   ```dart
   class DictionaryService {
     static Map<String, dynamic>? _cache;

     static Future<Map<String, dynamic>?> lookup(String word) async {
       if (_cache == null) {
         final jsonStr = await rootBundle.loadString('assets/data/arabic_dictionary.json');
         _cache = jsonDecode(jsonStr);
       }
       return _cache?[word.toUpperCase()];
     }
   }
   ```

### الخيار 2: استدعاء واجهة برمجة تطبيقات سحابية (REST API Lookup):
يمكن استدعاء API مجاني أو خاص لترجمة الكلمة وتوفير النطق والمعنى:
```dart
Future<void> fetchArabicMeaning(String word) async {
  final response = await http.get(
    Uri.parse('https://api.dictionaryapi.dev/api/v2/entries/en/$word'),
  );
  // أو استخدام خدمة ترجمة Google Translate / MyMemory API
}
```

### عرض بطاقة الكلمة الفورية (Vocabulary Modal / BottomSheet):
داخل `_onSelectionEnd` في `game_screen.dart`، قم بإظهار نافذة منبثقة تفاعلية:
```dart
if (result.status == SelectionStatus.found) {
  final word = result.word!;
  _showArabicVocabCard(context, word);
}

void _showArabicVocabCard(BuildContext context, String word) async {
  final data = await DictionaryService.lookup(word);
  if (!context.mounted) return;

  showModalBottomSheet(
    context: context,
    backgroundColor: Colors.transparent,
    builder: (ctx) => Container(
      padding: const EdgeInsets.all(24),
      decoration: const BoxDecoration(
        color: Color(0xFF1E293B),
        borderRadius: BorderRadius.vertical(top: Radius.circular(24)),
      ),
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Text(word, style: const TextStyle(fontSize: 26, fontWeight: FontWeight.bold, color: Colors.amber)),
          const SizedBox(height: 8),
          Text(
            data?['translation'] ?? 'جاري جلب الترجمة...',
            textDirection: TextDirection.rtl,
            style: const TextStyle(fontSize: 20, color: Colors.white),
          ),
          const SizedBox(height: 16),
          ElevatedButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text('متابعة اللعب'),
          )
        ],
      ),
    ),
  );
}
```

---

## 3. خطوات تشغيل المشروع (Running the Project)

مسار محرك Flutter المتوفر على جهازك: `C:\src\flutter\bin\flutter.bat`

### 1. الدخول لمجلد نواة اللعبة:
```powershell
cd C:\Users\ghenemi\Desktop\VocabCross_Flutter\game_core
```

### 2. تثبيت الحزم والمكتبات:
```powershell
& "C:\src\flutter\bin\flutter.bat" pub get
```

### 3. تشغيل اللعبة:
* **لتشغيلها على سطح المكتب (Windows):**
  ```powershell
  & "C:\src\flutter\bin\flutter.bat" run -d windows
  ```
* **لتشغيلها على المتصفح (Chrome/Web):**
  ```powershell
  & "C:\src\flutter\bin\flutter.bat" run -d chrome
  ```
* **لتشغيلها على جهاز Android موصول:**
  ```powershell
  & "C:\src\flutter\bin\flutter.bat" run -d android
  ```

