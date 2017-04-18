#include "stdafx.h"
#include <Magick++.h>
#include <zbar.h>
#include <zbar/Image.h>
#include <zbar/Exception.h>
#include "ZBarParser.h"

#define STR(s) #s

using namespace zbar;
using namespace Magick;

/** @brief 调用zbar执行二维码扫描
*
* filename 要扫描的文件
* QRResult 扫描的结果
*/
BOOL ZBarParseQRCode( string filename, string &QRResult )
{
	if( filename.empty() ) 
		return FALSE;

	const void *raw = NULL;
	int width = 0;
	int height = 0;
	// create a reader
	ImageScanner scanner;
	Magick::Blob blob;              // extract the raw data
	Magick::Image magick;  // read an image file
	
	try{
		magick.read( filename );

		if( !magick.isValid( ))
			return FALSE;

		width = magick.columns();   // extract dimensions
		height = magick.rows();
		magick.modifyImage();
		magick.write(&blob, "GRAY", 8);
		raw = blob.data();
	}
	catch( Magick::Exception &error_ ) 
	{
		printf("Caught exception: %s", error_.what() ); 
		return FALSE; 
	}

	if( raw == NULL || width == 0 || height == 0 )
		return FALSE;

	// configure the reader
	scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

	// wrap image data
	zbar::Image image(width, height, "Y800", raw, width * height  );

	// scan the image for barcodes
	try{
		int n = scanner.scan(image);
	}
	catch( zbar::Exception e )
	{
		printf("zbar scann image excption: %s", e.what() );
		return FALSE;
	}

	// extract results
	for(zbar::Image::SymbolIterator symbol = image.symbol_begin();	symbol != image.symbol_end();	++symbol) 
	{

		// do something useful with results
		printf( "decoded: %s symbol: %s", symbol->get_type_name().c_str(), symbol->get_data().c_str() );

		QRResult = symbol->get_data();
		break;
	}

	// clean up
	image.set_data(NULL, 0);

	return TRUE;
}