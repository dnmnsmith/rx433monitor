/* ===================================================
 * ClimeMetDecoder.h
 * ---------------------------------------------------
 * 433 Mhz decoding OoK frame from Climemet
 *
 *  Created on: 16 March 2015
 *  Author: Duncan Smith based on work by Dominique Pierre and Suat Özgür.
 *
 *	 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * 	 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * 	 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * 	 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * 	 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * 	 THE SOFTWARE.
 * ===================================================
 */


#include <stdint.h>
#include <iostream>
#include <stdio.h>

#include "ClimeMetDecoder.h"

static const int ClimeMetPacketSize = 48;



// Obscure but widely used code to produce LUT to reverse bits in a byte. 
#define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )

static const unsigned char BitReverseTable256[256] =
{
    R6(0), R6(2), R6(1), R6(3)
};

unsigned char reflect(unsigned char v)
{
    return BitReverseTable256[v];
}

unsigned char crclut[] = {

0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
0x9D, 0xC3, 0x21, 0x7F, 0xFC, 0xA2, 0x40, 0x1E,
0x5F, 0x01, 0xE3, 0xBD, 0x3E, 0x60, 0x82, 0xDC,
0x23, 0x7D, 0x9F, 0xC1, 0x42, 0x1C, 0xFE, 0xA0,
0xE1, 0xBF, 0x5D, 0x03, 0x80, 0xDE, 0x3C, 0x62,
0xBE, 0xE0, 0x02, 0x5C, 0xDF, 0x81, 0x63, 0x3D,
0x7C, 0x22, 0xC0, 0x9E, 0x1D, 0x43, 0xA1, 0xFF,
0x46, 0x18, 0xFA, 0xA4, 0x27, 0x79, 0x9B, 0xC5,
0x84, 0xDA, 0x38, 0x66, 0xE5, 0xBB, 0x59, 0x07,
0xDB, 0x85, 0x67, 0x39, 0xBA, 0xE4, 0x06, 0x58,
0x19, 0x47, 0xA5, 0xFB, 0x78, 0x26, 0xC4, 0x9A,
0x65, 0x3B, 0xD9, 0x87, 0x04, 0x5A, 0xB8, 0xE6,
0xA7, 0xF9, 0x1B, 0x45, 0xC6, 0x98, 0x7A, 0x24,
0xF8, 0xA6, 0x44, 0x1A, 0x99, 0xC7, 0x25, 0x7B,
0x3A, 0x64, 0x86, 0xD8, 0x5B, 0x05, 0xE7, 0xB9,
0x8C, 0xD2, 0x30, 0x6E, 0xED, 0xB3, 0x51, 0x0F,
0x4E, 0x10, 0xF2, 0xAC, 0x2F, 0x71, 0x93, 0xCD,
0x11, 0x4F, 0xAD, 0xF3, 0x70, 0x2E, 0xCC, 0x92,
0xD3, 0x8D, 0x6F, 0x31, 0xB2, 0xEC, 0x0E, 0x50,
0xAF, 0xF1, 0x13, 0x4D, 0xCE, 0x90, 0x72, 0x2C,
0x6D, 0x33, 0xD1, 0x8F, 0x0C, 0x52, 0xB0, 0xEE,
0x32, 0x6C, 0x8E, 0xD0, 0x53, 0x0D, 0xEF, 0xB1,
0xF0, 0xAE, 0x4C, 0x12, 0x91, 0xCF, 0x2D, 0x73,
0xCA, 0x94, 0x76, 0x28, 0xAB, 0xF5, 0x17, 0x49,
0x08, 0x56, 0xB4, 0xEA, 0x69, 0x37, 0xD5, 0x8B,
0x57, 0x09, 0xEB, 0xB5, 0x36, 0x68, 0x8A, 0xD4,
0x95, 0xCB, 0x29, 0x77, 0xF4, 0xAA, 0x48, 0x16,
0xE9, 0xB7, 0x55, 0x0B, 0x88, 0xD6, 0x34, 0x6A,
0x2B, 0x75, 0x97, 0xC9, 0x4A, 0x14, 0xF6, 0xA8,
0x74, 0x2A, 0xC8, 0x96, 0x15, 0x4B, 0xA9, 0xF7,
0xB6, 0xE8, 0x0A, 0x54, 0xD7, 0x89, 0x6B, 0x35 };

unsigned char crc8(unsigned char poly, unsigned char crc,  unsigned const char *data, size_t len)
{
    unsigned const char *end;

    if (len == 0)
        return crc;

    end = data + len;
    do {
          crc = crclut[  crc ^ reflect( *data++ ) ];
    } while (data < end);
    
    return reflect(crc);
}


ClimeMetDecoder::ClimeMetDecoder( IResultNotifier * pClient ) : m_pClient( pClient ) 
{

}

int ClimeMetDecoder::decode (word width)
{
 if (width < 250)
 {
//      std::cerr << "Pulse too narrow" << std::endl;
  return -1;
 }
 else if (width < 750)  // 1; possible start of 12 pair for one
 {
  //std::cerr << "PW1" << std::endl;
  switch (state)   
  {
   case UNKNOWN:  
        state = T1; 
        break; // Tentative 1 state 
   
   case OK:            
        if (total_bits == (ClimeMetPacketSize-1)) // Looks like last bit? Final component will be malformed.
        {
         //std::cerr << "Got last 1" << std::endl; 
         gotBit(1);
         return finished();
        }
        else
        {
         state = T1;  // Tentative 1 state
        } 
        break;

   default:
        return -1;   // Bzzzt. Fail. Expected length 2
        
   }
 }
 else if (width < 1250) // 2; possible end of 12 pair for one or 32 pair for 0
 {
  //std::cerr << "PW2" << std::endl;
  switch (state) 
  {
   case T0: 
        gotBit(0); 
        //std::cerr << "Got bit 0" << std::endl;
        break;
   case T1: 
        //std::cerr << "Got bit 1" << std::endl;
        gotBit(1); 
        break;
   default:
        return -1;      // Not expected as first of pair.
   }
 }
 else if (width < 1750) // 3; possible start of 32 pair for zero
 {
  //std::cerr << "PW3" << std::endl;
  switch (state) 
  {
   case OK:
        if (total_bits == (ClimeMetPacketSize-1)) // Looks like last bit? Final zero will be malformed.
        {
         //std::cerr << "Got last 0" << std::endl; 
         gotBit(0);
         return finished();
        }
        else
         state = T0;   // Tentative 0 state
        break;
   default:
        return -1;   // Bzzzt. Fail. Expected length 2
    
   }
 }
 else 
 {
//      std::cerr << "Pulse too wide" << std::endl;
  return -1;
 } 
return 0;   // Continues
}
    
// Check and return state
int ClimeMetDecoder::finished()
{
 reverseBits();

 // First byte should be all ones sync.
 if (data[0] != 255)
    return -1;
             
 int result = VerifyCheckSum();
 
 if (result != 1)
 {
//    std::cerr << "Checksum error" << std::endl;
    return -1;  
 }
    
 return 1;
}


void ClimeMetDecoder::notify()
{
 if (m_pClient == NULL)
    return;
    
 if (!isDone())
    return;
   
 int code = (data[1] << 4) | ((data[2] & 0xF0) >> 4); 
 int t =  (((data[ 2 ]) & 0x07) << 8) | (data[ 3 ]);
 bool neg = (((data[ 2 ]) & 8) != 0);  
 float temp = static_cast<float>( t ) / 10.0 * (neg ? -1 : 1);

 char bufTemp[ 40 ];
 sprintf( bufTemp, "%02.1f",temp );
 
 char bufCode[ 40 ];
 sprintf( bufCode, "%03X",code );
 
 m_pClient->notify( std::string( bufCode ), "Temperature", std::string( bufTemp ) );
 
 if (data[ 4 ] != 255)
 {
   char bufHumidity[ 40 ];
   sprintf( bufHumidity, "%d",static_cast<int>(data[ 4 ]) );
   m_pClient->notify( std::string( bufCode ), "Humidity", std::string( bufHumidity ) );
 }
}

bool ClimeMetDecoder::getTemperature( float &temp ) const
{
 if (isDone())
 {
   int t =  (((data[ 2 ]) & 0x07) << 8) | (data[ 3 ]);
   bool neg = (((data[ 2 ]) & 8) != 0);
   
   temp = static_cast<float>( t ) / 10.0 * (neg ? -1 : 1);

   return true;
 }
 
 return false;
}

bool ClimeMetDecoder::getHumidity( int &humidity ) const
{
 if (isDone() && (data[ 4 ] != 255))
 {
  humidity = static_cast<int>(data[ 4 ]);
  return true;
 }
 return false;
}

bool ClimeMetDecoder::getDeviceCode( int &code ) const
{
 if (isDone())
 {
   code = (data[1] << 4) | ((data[2] & 0xF0) >> 4); 
   return true;
 }
 
 return false;
}

int ClimeMetDecoder::VerifyCheckSum() const
{
   unsigned char poly = 0x8C;
   unsigned char init = 0xFF;
   unsigned char crc = crc8( poly, init, data, pos-1);
   return (crc==data[pos-1]) ? 1 : -1;
}
