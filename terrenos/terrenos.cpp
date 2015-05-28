#include <stdio.h>
#include <stdlib.h>
#include <libnoise/noise.h>
#include "util.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>

cv::Vec4f water( 0.75f, 0, 0, 1.f ),
			low( 0.25, 0.5f, 0.5f, 1.f ),
			mid( 0.125f, 0.25f, 0.125f,1.f),
			high( 0.0625, 0.125, 0.0625,1);

cv::Vec4f biomes[9];

void InitBiomes()
{
	int n = 0;
	for( int hum = 0; hum < 3; hum++ )
	{
		for( int alt = 0; alt < 3; alt++ )
		{
			int h, a;
			h = hum + 1;
			a = alt + 1;
			biomes[n] = cv::Vec4f( 0, h * 0.05 + a * 0.05, 0, 1.f);
			n++;
		}
	}
	// humedad 0
	biomes[0] = cv::Vec4f(0.4, 0.7, 0.8,1); biomes[1] = cv::Vec4f( 0.3, 0.4, 0.3,1 ); biomes[2] = cv::Vec4f( 0.6, 0.6, 0.6,1 );
	// humedad 1
	biomes[3] = cv::Vec4f(0.5, 0.6, 0.5,1); biomes[4] = cv::Vec4f( 0.4, 0.4, 0.1,1 ); biomes[5] = cv::Vec4f( 0.8, 0.7, 0.7,1 );
	// humedad 2
	biomes[6] = cv::Vec4f(0.5, 0.6, 0.2,1); biomes[7] = cv::Vec4f( 0.4 , 0.4, 0.1,1 ); biomes[8] = cv::Vec4f( 0.8, 0.7, 0.7,1 );
}

cv::Vec4f GetBiome(int hum, int alt)
{
	return biomes[alt + hum * 3];
}

#define THRESHOLD_WATER -0.3
#define THRESHOLD_LOW 0.1
#define THRESHOLD_MID 0.5

#define loopxy(cc,rr) for( int r = 0; r < rr; r++ ) \
					  for( int c = 0; c < cc; c++ )

cv::Mat GenPerlin( cv::Size sz, float freq, float oct_count, float scale );

cv::Mat GetWater( const cv::Mat& perlin )
{
	cv::Mat ret(cv::Size(perlin.cols, perlin.rows), CV_32F);
	loopxy(perlin.cols, perlin.rows)
	{
		float v = perlin.at<float>(c,r);
		ret.at<float>(c,r) = ( v < THRESHOLD_WATER ) ? 1.f : 0.f;
	}
	return ret;
}

cv::Mat ColourHeights( const cv::Mat& perlin,
					   float thrsh_water = THRESHOLD_WATER,
					   float thrsh_low = THRESHOLD_LOW,
					   float thrsh_mid = THRESHOLD_MID )
{
	cv::Mat h(cv::Size(perlin.cols, perlin.rows), CV_32FC4);
	for( int x = 0; x < perlin.cols; x++ )
	for( int y = 0; y < perlin.rows; y++ )
	{
		float v = perlin.at<float>(y,x);
		cv::Vec4f t;
		if( v < thrsh_water ) t = water;
		else if( v < thrsh_low ) t = low;
		else if( v < thrsh_mid ) t = mid;
		else t = high;
		h.at<cv::Vec4f>(y,x) = t;
	}
	return h;
}

cv::Mat GenPerlin( int width, int height, float freq, float oct_count, float scale )
{
	cv::Mat ret(cv::Size(width, height), CV_32F);

	noise::module::Perlin themod;
	themod.SetFrequency(freq);
	themod.SetOctaveCount(oct_count);

	for( int x = 0; x < width; x++ )
	for( int y = 0; y < height; y++ )
	{
		double v = themod.GetValue( x * scale / width, y * scale / height, 0 );
		ret.at<float>(y,x) = v;
	}
	return ret;
}

cv::Mat Resized( const cv::Mat& src, float factor )
{
	cv::Mat resized;
	cv::resize( src, resized, cv::Size( src.rows * factor, src.cols * factor ) );
	return resized;
}

int n = 0;

void Show( const cv::Mat& src, const char* text = "NOTEXT", float scale=1.f/4.f )
{
	std::stringstream ss;
	ss << "im" << n << ".png";
	cv::imwrite( ss.str(), src );

	cv::Mat m = Resized( src, scale );
	cv::putText( m, text, cv::Point(0,200), cv::FONT_HERSHEY_SIMPLEX, 1, CV_RGB(255,0,255), 4 );
	cv::namedWindow( "CvWin", cv::WINDOW_AUTOSIZE );
	cv::imshow( "CvWin", m );
	cv::waitKey(0);
	n++;
}

cv::Mat Expand( const cv::Mat& src )
{
	cv::Mat ret( src.cols, src.rows, CV_32F);
	double min, max;
	cv::minMaxLoc( src, &min, &max );
	printf("min: %f\nmax: %f\n", min,max);
	ret = (src - min) / (max - min) * 1;
	cv::minMaxLoc( src, &min, &max );
	printf("min: %f\nmax: %f\n\n", min,max);
	return ret;
}


cv::Mat GaussBlur( const cv::Mat& src, int size, int sigma )
{
	cv::Mat dst;
	if( size%2 == 0 ) size++;
	cv::GaussianBlur( src, dst, cv::Size(size,size), sigma, sigma );
	return Expand(dst);
}

const float h1 = 0.02;
const float h2 = 0.2;


cv::Mat Alphaize( const cv::Mat& src )
{
	cv::Mat ret(cv::Size(src.cols, src.rows), CV_32FC4);
	loopxy(src.cols, src.rows)
	{
		float v = src.at<float>(r,c);
		if( v < h1 ) v = 0;
		else if( v < h2 ) v = 0.5;
		else v = 1;
		ret.at<cv::Vec4f>(r,c) = cv::Vec4f(v,0,0,1);
	}
	return ret;
}

cv::Mat Biomize( const cv::Mat& heights, const cv::Mat& humedad )
{
	cv::Mat ret( heights.cols, heights.rows, CV_32FC4 );
	loopxy(heights.cols, heights.rows)
	{
		float hum = humedad.at<float>(r,c);
		if( hum < h1 ) hum = 0;
		else if( hum < h2 ) hum = 1;
		else hum = 2;
		cv::Vec4f alt = heights.at<cv::Vec4f>(r,c);
		if( alt == water )
		{
			ret.at<cv::Vec4f>(r,c) = water;
		}
		else
		{
			int al;
			if( alt == low ) al = 0;
			if( alt == mid ) al = 1;
			if( alt == high ) al = 2;
			cv::Vec4f col = GetBiome( hum, al );
			ret.at<cv::Vec4f>(r,c) = col;
		}

		/*
			// HUMEDAD BAJA
			if( hum < h1 )
			{
				if( alt == low ) col = cv::Vec4f(1,1,1,1);
				else if( alt == mid ) col = cv::Vec4f(1,1,1,1);
				else if( alt == high ) col = cv::Vec4f(1,1,1,1);
			}
			// HUMEDAD MEDIA
			else if( hum < h2 )
			{
				if( alt == low ) col = cv::Vec4f(1,1,1,1);
				else if( alt == mid ) col = cv::Vec4f(1,1,1,1);
				else if( alt == high ) col = cv::Vec4f(1,1,1,1);

			}
			// HUMEDAD ALTA
			else
			{
				if( alt == low ) col = cv::Vec4f(1,1,1,1);
				else if( alt == mid ) col = cv::Vec4f(1,1,1,1);
				else if( alt == high ) col = cv::Vec4f(1,1,1,1);
			}
		}
		*/
	}
	return ret;
}

cv::Mat Add( const cv::Mat& m1, const cv::Mat& m2, float f1 = 1.f, float f2=1.f, float gamma = 0.f )
{
	cv::Mat ret;
	cv::addWeighted( m1, f1, m2, f2, gamma, ret );
	return ret;
}


int main( int argc, char** argv )
{
	InitBiomes();
	bool force = false;
	const char* DATA_FILE = "/home/razieliyo/MAPGENDATA.xml";
	int width, height;
	cv::Mat continentes_perlin, lagos_perlin;
	width = height = 3456;

	printf("%d\n",argc);
	if( argc == 2 )
	{
		width = height = atoi(argv[1]);
		force = true;
	}

	if( force || access( DATA_FILE, F_OK ) == -1 )
	{
		// Generamos perlines
		cv::FileStorage file(DATA_FILE, cv::FileStorage::WRITE);
		{
			Task _("Generando perlines");
			continentes_perlin = GenPerlin( width, height, width / 32, 10, 0.08 );
			lagos_perlin = GenPerlin( width, height, width / 32, 5, 0.8 );
		}
		file << "continentes_perlin" << continentes_perlin;
		file << "lagos_perlin" << lagos_perlin;
	}
	else
	{
		// Leemos perlines
		cv::FileStorage file;
		{
		Task _("Cargar datos");
		file = cv::FileStorage(DATA_FILE, cv::FileStorage::READ);
		}
		{
		Task _("Cargar perlin continentes");
		file["continentes_perlin"] >> continentes_perlin;
		}
		{
		Task _("Cargar perlin lagos");
		file["lagos_perlin"] >> lagos_perlin;
		}
		width = lagos_perlin.cols;
		height = lagos_perlin.rows;
	}

	cv::Mat continentes, lagos;
	{
		Task _("Colorear alturas");
		continentes = ColourHeights( continentes_perlin );
		lagos = ColourHeights( lagos_perlin, -0.7, 0, 0 );
	}

	cv::Mat addition;
	{
		Task _("Unir alturas");
		cv::addWeighted( continentes_perlin, 0.8, lagos_perlin, 0.2, 0.0, addition );
	}

	cv::Mat def(addition.cols, addition.rows, CV_32F);
	{
		Task _("Capar con continentes");
		for( int x = 0; x < width; x++ )
		for( int y = 0; y < height; y++ )
		{
			if( continentes.at<cv::Vec4f>(y,x) == water )
			{
				def.at<float>(y,x) = -1;
			}
			else
			{
				def.at<float>(y,x) = addition.at<float>(y,x);
			}
		}
	}

	int gauss_spread_lagos = 50;
	int gauss_spread_continentes = 100;
	int gauss_size_lagos = 1024;
	int gauss_size_continentes = 1024;
	float lagos_humedad_factor = 0.4;
	float continentes_humedad_factor = 0.4;

	cv::Mat lagos_water = GetWater( lagos_perlin );
	cv::Mat lagos_gauss = GaussBlur( lagos_water, gauss_size_continentes, gauss_spread_lagos );
	cv::Mat lagos_humedad = Add(Alphaize(lagos_gauss), lagos, 1, 1);

	cv::Mat continentes_water = GetWater( continentes_perlin );
	cv::Mat continentes_gauss = GaussBlur( continentes_water, gauss_size_continentes, gauss_spread_continentes );
	cv::Mat continentes_humedad = Add(Alphaize(continentes_gauss), continentes, 1, 1);

	cv::Mat suma_water = GetWater( def );
	cv::Mat suma_gauss = GaussBlur( suma_water, 1023, 100 );
	//cv::Mat suma_humedad = Add(Alphaize(continentes_gauss)*continentes_humedad_factor,Alphaize(lagos_gauss)*lagos_humedad_factor,1,1);
	cv::Mat suma_humedad = Add(Alphaize(suma_gauss), ColourHeights( def ), 1,1);

	cv::Mat biom = Biomize( ColourHeights(def), suma_gauss);

	Show( lagos, "Lagos" );
	Show( lagos_water, "Agua lagos" );
	Show( lagos_gauss, "Gauss lagos" );
	Show( lagos_humedad, "Humedad lagos");

	Show( continentes, "Continentes" );
	Show( continentes_water, "Agua continentes" );
	Show( continentes_gauss, "Gauss continentes" );
	Show( continentes_humedad, "Humedad continentes" );

	Show( lagos, "Lagos" );
	Show( continentes, "Continentes" );
	Show( ColourHeights( addition ), "Suma" );
	Show( ColourHeights( def ), "Final" );
	//Show( ColourHeights( Add(suma_humedad, ColourHeights( def ), 1, 1 ) ) );
	Show( suma_gauss, "Gauss suma" );
	Show( suma_humedad, "Suma humedad" );
	Show( biom, "Biomas" );

	cv::imwrite("kkk.png", biom*255);

	return 0;
}

