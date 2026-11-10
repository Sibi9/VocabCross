import json

p = 'assets/data/bac_vocabulary.json'
with open(p, encoding='utf-8') as f:
    data = json.load(f)

units = data['units']

# Unit 1: align the flagship crossword level with the reference word BRIBERY.
for w in units[0]['words']:
    if w['word'] == 'BRIBE':
        w['word'] = 'BRIBERY'
        w['definition_en'] = 'The practice of offering money for illegal favor'
        w['translation_ar'] = 'رشوة / ارتشاء'
        w['synonym_en'] = 'Payoff, graft'
        w['letters'] = list('BRIBERY')
        w['image_keyword'] = 'bribery_money'
        w['difficulty'] = 2
        w['frequency'] = 'high'

# Unit 3: swap CELESTIAL for the ECLIPSE level used by the image-guess reference.
for w in units[2]['words']:
    if w['word'] == 'CELESTIAL':
        w['word'] = 'ECLIPSE'
        w['definition_en'] = 'An astronomical event in which one celestial body obscures another'
        w['translation_ar'] = 'كسوف / خسوف'
        w['synonym_en'] = 'Occultation, obscuration'
        w['letters'] = list('ECLIPSE')
        w['image_keyword'] = 'eclipse'
        w['difficulty'] = 3
        w['frequency'] = 'high'
        w['mode'] = 'image'

# Ensure letters arrays are exact and modes are valid.
for u in units:
    for w in u['words']:
        w['letters'] = list(w['word'])
        assert w['mode'] in ('wheel', 'crossword', 'image'), w
        assert len(w['letters']) == len(w['word'])

with open(p, 'w', encoding='utf-8') as f:
    json.dump(data, f, ensure_ascii=False, indent=4)

count = 0
for u in units:
    print(u['id'], len(u['words']), [w['word'] + ':' + w['mode'] for w in u['words']])
    count += len(u['words'])
print('TOTAL', count)
