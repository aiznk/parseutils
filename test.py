import parseutils as pu
import unittest

class Test(unittest.TestCase):
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
