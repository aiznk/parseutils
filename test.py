import parseutils as pu
import unittest

class Test(unittest.TestCase):
	def test_parse_tag(self):
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
		self.kv_eq('abc=def', 7, 'abc', 'def')
		self.kv_eq('_a-b_c=_d-e_f', 13, '_a-b_c', '_d-e_f')
		self.kv_eq('abc = def', 9, 'abc', 'def')
		self.kv_eq('abc  =   def', 12, 'abc', 'def')
		self.kv_eq('   abc  =   def   ', 15, 'abc', 'def')
		self.kv_eq('abc="def"', 9, 'abc', 'def')
		self.kv_eq('abc="  def  "', 13, 'abc', '  def  ')
		self.kv_eq('abc="  \\"def  "', 15, 'abc', '  "def  ')

unittest.main()
