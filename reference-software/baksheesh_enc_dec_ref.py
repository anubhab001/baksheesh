
# SBox
LS_sbox = 3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1
LS_inv_sbox = 1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9

# Bit-permutation
player = 0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31
inv_player = 0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115

# Round constants
RC = 2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8
const_loc = 8, 13, 19, 35, 67, 106

# Round number
N_rounds = 35

def sbox_layer(pt, sbox):
	ct = [None]*len(pt)
	for i in range(len(pt)):
		ct[i] = sbox[pt[i]]
	return ct[:]


def bits_to_nibble(x):
	assert len(x) == 4
	y = x[3]*1 + x[2]*2 + x[1]*4 + x[0]*8
	return y

def bits_to_nibbles(x):
	y = []
	for i in range(len(x)//4):
		z = x[4*i : 4*i + 4]
		y.append(bits_to_nibble(z))
	return y

def nibble_to_bits(x):
	y = []
	z = [zz == '1' for zz in (bin(x)[2:]).zfill(4)]
	y.extend(z)
	return y

def nibbles_to_bits(x):
	y = map(nibble_to_bits, x)
	return [item for sublist in y for item in sublist]

def nibble_encoding(a):
	assert [type(aa) == type(True) for aa in a]
	a_nibbles = bits_to_nibbles(a)

	b = []
	for aa in a_nibbles:
		b.append(bits_to_nibble(nibble_to_bits(aa)[::-1]))

	return nibbles_to_bits(b)

def key_update(key, step=1):
	# Right rotate the entire key 1 step
	#print ('Key', pretty_print(key))
	temp_key = nibbles_to_bits(key)
	temp_key = temp_key[+step:] + temp_key[:+step]
	key = bits_to_nibbles(temp_key)
	return key

#def key_update(key, step=1):
#	# Left rotate the entire key 1 step
#	temp_key = nibbles_to_bits(key)
#	temp_key = temp_key[-step:] + temp_key[:-step]
#	key = bits_to_nibbles(temp_key)
#	return key

def add_round_constants(state, rc):
	state[const_loc[0]] ^= (rc & 1)
	state[const_loc[1]] ^= ((rc >> 1) & 1)
	state[const_loc[2]] ^= ((rc >> 2) & 1)
	state[const_loc[3]] ^= ((rc >> 3) & 1)
	state[const_loc[4]] ^= ((rc >> 4) & 1)
	state[const_loc[5]] ^= ((rc >> 5) & 1)
	#state[const_loc[6]] ^= True
	return state

def perm_layer(x, player):
	assert len(x) == 128
	y = [None]*len(x)
	for i in range(len(x)):
		y[player[i]] = x[i]
	return y[:]

def add_key(pt, key, xor_cover=1.0):
	if xor_cover == 1.0: # Full round key XOR
		return [(p^k) for p, k in zip(pt, key)]

	elif xor_cover == 0.75: # Three-quarter round key XOR (bit corresponding to 0x8 skipped from key bit XOR)
		return [(p)^(k & 0b1101) for p, k in zip(pt, key)]
	
	elif xor_cover == 0.5: # Half round-key XOR
		return [(p)^(k & 0b0110) for p, k in zip(pt, key)]
	
	elif xor_cover == 0.0: # Unkeyed
		return [(p) for p, k in zip(pt, key)]
	
	else:
		raise NotImplementedError
	
def pretty_print(x):
	y = []
	for xx in x:
		y.append(hex(xx)[2:])
	return (''.join(y))[::-1]


def encrypt(key, pt, no_of_rounds=N_rounds, xor_cover=1.0):
	# round key add
	pt = add_key(pt, key, xor_cover=xor_cover)

	for r in range(no_of_rounds):
		# sbox
		pt = sbox_layer(pt=pt, sbox=LS_sbox)

		# permutation
		pt = nibbles_to_bits(pt)
		pt = perm_layer(x=pt, player=player)

		# round constants
		pt = add_round_constants(state=pt, rc=RC[r])
		pt = bits_to_nibbles(pt)

		# Run key update
		key = key_update(key, step=1)

		# round key add
		pt = add_key(pt, key, xor_cover=xor_cover)
		#print (r, pretty_print(key))

	return pt

def decrypt(key, pt, no_of_rounds=N_rounds, xor_cover=1.0):
	key = key_update(key, step=N_rounds)
	for r in range(no_of_rounds)[::-1]:

		# round key add
		pt = add_key(pt, key, xor_cover=xor_cover)

		# Run key update
		key = key_update(key, step=-1)

		# round constants
		pt = nibbles_to_bits(pt)
		pt = add_round_constants(state=pt, rc=RC[r])

		# inv permutation
		pt = perm_layer(x=pt, player=inv_player)
		pt = bits_to_nibbles(pt)

		# inv sbox
		pt = sbox_layer(pt=pt, sbox=LS_inv_sbox)


	# Run key update
	key = key_update(key, step=0)[:]
	# round key add
	pt = add_key(pt, key, xor_cover=xor_cover)

	return pt


if __name__ == '__main__':

	key = [0]*32
	pt = [0]*32 
	
	# xor_cover = 1.0 ################ Full key

	# print (len(key))
	# print('Encryption:')
	# print ('Key', pretty_print(key))
	# print ('PT ', pretty_print(pt))
	# pt_orig = pt[:]

	# assert len(pt) == 32
	# assert len(key) == 32
	# assert N_rounds == 35, 'Incorrect number of rounds'
	# ct = encrypt(key=key[:], pt=pt[:], no_of_rounds=N_rounds, xor_cover=xor_cover)
	# print ('CT ', pretty_print(ct))

	# print('Decryption:')
	# print ('Key', pretty_print(key))
	# print ('CT ', pretty_print(ct))
	# pt = decrypt(key=key[:], pt=ct[:], no_of_rounds=N_rounds, xor_cover=xor_cover)
	# print ('PT ', pretty_print(pt))
	# assert pt == pt_orig, 'Decrypted ciphertext not matching with plaintext'


	xor_cover = 0.75 ################ Three-quarter key

	print (len(key))
	print('Encryption:')
	print ('Key', pretty_print(key))
	print ('PT ', pretty_print(pt))
	pt_orig = pt[:]

	assert len(pt) == 32
	assert len(key) == 32
	assert N_rounds == 35, 'Incorrect number of rounds'
	ct = encrypt(key=key[:], pt=pt[:], no_of_rounds=N_rounds, xor_cover=xor_cover)
	print ('CT ', pretty_print(ct))

	print('Decryption:')
	print ('Key', pretty_print(key))
	print ('CT ', pretty_print(ct))
	pt = decrypt(key=key[:], pt=ct[:], no_of_rounds=N_rounds, xor_cover=xor_cover)
	print ('PT ', pretty_print(pt))
	assert pt == pt_orig, 'Decrypted ciphertext not matching with plaintext'


# """
# Key 00000000000000000000000000000000
# PT  00000000000000000000000000000000
# CT  5b447b7ad394ad8650e90664c6bca198
# """