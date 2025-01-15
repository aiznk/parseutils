import parseutils as pu
import unittest

class Test(unittest.TestCase):
	def test_parse_section(self):
		src = '[123]'
		j, section_name = pu.parse_section(0, src, len(src))
		self.assertEqual(j, 5)
		self.assertEqual(section_name, '123')
		src = '[  abc.def  ]'
		j, section_name = pu.parse_section(0, src, len(src))
		self.assertEqual(j, 13)
		self.assertEqual(section_name, 'abc.def')

	def test_parse_tag(self):
		src = '<div href="hige" class=\'myclass\' id=123>content</div>'
		j, tag_name, tag_type, attrs = pu.parse_tag(0, src, len(src))
		self.assertEqual(j, 40)
		self.assertEqual(tag_name, 'div')
		self.assertEqual(tag_type, 'begin')
		self.assertEqual(attrs['href'], 'hige')
		self.assertEqual(attrs['class'], 'myclass')
		self.assertEqual(attrs['id'], '123')
		j, tag_name, tag_type, attrs = pu.parse_tag(j, src, len(src))
		self.assertEqual(j, 53)
		self.assertEqual(tag_name, 'div')
		self.assertEqual(tag_type, 'end')

		src = '<a href="hige" class=\'myclass\' id=1>content</a>'
		j, tag_name, tag_type, attrs = pu.parse_tag(0, src, len(src))
		self.assertEqual(j, 36)
		self.assertEqual(tag_name, 'a')
		self.assertEqual(tag_type, 'begin')
		self.assertEqual(attrs['href'], 'hige')
		self.assertEqual(attrs['class'], 'myclass')
		self.assertEqual(attrs['id'], '1')
		j, tag_name, tag_type, attrs = pu.parse_tag(j, src, len(src))
		self.assertEqual(j, 47)
		self.assertEqual(tag_name, 'a')
		self.assertEqual(tag_type, 'end')

	def test_parse_css_blocks(self):
		src = """
:root > #hoge {
	font-size: 1rem;
}
"""
		j, blocks = pu.parse_css_blocks(0, src, len(src))
		self.assertEqual(blocks[':root > #hoge']['font-size'], '1rem')

		src = """
@media (max-width: 100px) {
	div {
		font-size: 1rem;
	}
	p {
		padding: 2rem;
	}
}
"""
		j, blocks = pu.parse_css_blocks(0, src, len(src))
		self.assertEqual(j, 85)
		self.assertEqual(blocks['@media (max-width: 100px)']['div']['font-size'], '1rem')
		self.assertEqual(blocks['@media (max-width: 100px)']['p']['padding'], '2rem')

		src = """
* { font-size: 1rem; }
"""
		j, blocks = pu.parse_css_blocks(0, src, len(src))
		self.assertEqual(j, 24)
		self.assertEqual(blocks['*']['font-size'], '1rem')

		src = """
div {
	margin: 1rem 2rem;
	padding: 1rem;
}
p { font-size: 1rem; }

header {
	background: var(--color-bg);
}
"""
		j, blocks = pu.parse_css_blocks(0, src, len(src))
		self.assertEqual(j, 110)
		self.assertEqual(blocks['div']['margin'], '1rem 2rem')
		self.assertEqual(blocks['div']['padding'], '1rem')
		self.assertEqual(blocks['p']['font-size'], '1rem')
		self.assertEqual(blocks['header']['background'], 'var(--color-bg)')

	def test_parse_css_block(self):
		src = """a {}    """
		j, ident, block = pu.parse_css_block(0, src, len(src))
		self.assertEqual(j, 4)
		self.assertEqual(ident, 'a')

		src = """
.class-name {
    background: red;
    padding: 1rem 2rem;
}
    """
		j, ident, block = pu.parse_css_block(0, src, len(src))
		self.assertEqual(j, 61)
		self.assertEqual(ident, '.class-name')
		self.assertEqual(block['background'], 'red')
		self.assertEqual(block['padding'], '1rem 2rem')

	def kv_eq(self, src, i, key, val):
		j, k, v = pu.parse_key_value(0, src, len(src))
		self.assertEqual(j, i)
		self.assertEqual(k, key)
		self.assertEqual(v, val)

	def test_parse_key_value(self):
		self.kv_eq('abc = 31.24', 11, 'abc', '31.24')
		self.kv_eq('abc=31.24', 9, 'abc', '31.24')
		self.kv_eq('abc=def', 7, 'abc', 'def')
		self.kv_eq('_a-b_c=_d-e_f', 13, '_a-b_c', '_d-e_f')
		self.kv_eq('abc = def', 9, 'abc', 'def')
		self.kv_eq('abc  =   def', 12, 'abc', 'def')
		self.kv_eq('   abc  =   def   ', 15, 'abc', 'def')
		self.kv_eq('abc="def"', 9, 'abc', 'def')
		self.kv_eq('abc="  def  "', 13, 'abc', '  def  ')
		self.kv_eq('abc="  \\"def  "', 15, 'abc', '  "def  ')
		self.kv_eq("abc='def'", 9, 'abc', 'def')

if __name__ == '__main__':
	unittest.main()
