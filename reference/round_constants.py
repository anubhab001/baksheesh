#!/usr/bin/env python3
"""
BAKSHEESH Round Constants Generator 

CORRECTED Hardware Logic:
- 5-bit LFSR with right-shift operation
- Feedback polynomial: x^5 x^2 + 1 (taps at positions 2 and 0)  
- Initialization: 00001
- Toggle bit at position 0 (LSB)
- Round constant = (LFSR << 1) | toggle
"""

def generate_baksheesh_round_constants_lfsr():
    round_constants = []
    
    # Initialize LFSR to 00001 (as per specification)
    lfsr = 0b00001
    toggle = 0  # Start with toggle = 0
    
    for round_num in range(35):
        # Construct 6-bit round constant: LFSR bits at [5:1], toggle at [0]
        round_constant = (lfsr << 1) | toggle
        round_constants.append(round_constant)
        
        print(f"Round {round_num:2d}: LFSR=0b{lfsr:05b}, Toggle={toggle}, RC={round_constant:2d}")
        
        # Update LFSR using right-shift with feedback polynomial x^2 + 1
        feedback = ((lfsr >> 2) & 1) ^ ((lfsr >> 0) & 1)  # x^2 + x^0
        lfsr = (feedback << 4) | (lfsr >> 1)  # Right shift, feedback to MSB
        lfsr &= 0x1F  # Keep 5 bits
        
        # Toggle bit alternates each round
        toggle = 1 - toggle
    
    return round_constants




if __name__ == "__main__":
    generate_baksheesh_round_constants_lfsr()