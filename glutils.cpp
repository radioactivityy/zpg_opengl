#include "glutils.h"

/* OpenGL check state */
bool check_gl( const GLenum error )
{
	if ( error != GL_NO_ERROR )
	{
		//const GLubyte * error_str;
		//error_str = gluErrorString( error );
		//printf( "OpenGL error: %s\n", error_str );

		return false;
	}

	return true;
}

/* glfw callback */
void glfw_callback( const int error, const char * description )
{
	printf( "GLFW Error (%d): %s\n", error, description );
}

/* OpenGL messaging callback */
void GLAPIENTRY gl_callback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void * user_param )
{
	printf( "GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		( type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message" ),
		type, severity, message );
}

/* invoked when window is resized */
void framebuffer_resize_callback( GLFWwindow * window, int width, int height )
{
	glViewport( 0, 0, width, height );
}

/* load shader code from the text file */
int LoadShader( const std::string & file_name, std::vector<char> & shader )
{
	FILE * file = fopen( file_name.c_str(), "rt" );

	if ( !file )
	{
		printf( "IO error: File '%s' not found.\n", file_name.c_str() );

		return S_FALSE;
	}

	int result = S_FALSE;

	/*std::ifstream in(file_name, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();*/
	std::filesystem::path p{ file_name };
	const auto file_size = std::filesystem::file_size( p );
	//const size_t file_size = static_cast< size_t >( GetFileSize64( file_name.c_str() ) );

	if ( file_size < 1 )
	{
		printf( "Shader error: File '%s' is empty.\n", file_name.c_str() );
	}
	else
	{
		/* in glShaderSource we don't set the length in the last parameter,
		so the string must be null terminated, therefore +1 and reset to 0 */
		shader.clear();
		shader.resize( file_size + 1 );

		size_t bytes = 0; // number of already loaded bytes

		do
		{
			bytes += fread( shader.data(), sizeof( char ), file_size, file );
		} while ( !feof( file ) && ( bytes < file_size ) );

		if ( !feof( file ) && ( bytes != file_size ) )
		{
			printf( "IO error: Unexpected end of file '%s' encountered.\n", file_name.c_str() );
		}
		else
		{
			printf( "Shader file '%s' loaded successfully.\n", file_name.c_str() );
			result = S_OK;
		}
	}

	fclose( file );
	file = nullptr;

	return result;
}

std::string LoadAsciiFile( const std::string & file_name )
{
	std::ifstream file( file_name, std::ios::in );

	if ( file )
	{
		return ( std::string( ( std::istreambuf_iterator<char>( file ) ), std::istreambuf_iterator<char>() ) );
	}
	else
	{
		return "";
	}
}

/* check shader for completeness */
GLint CheckShader( const GLenum shader )
{
	GLint status = 0;
	glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

	printf( "Shader compilation %s.\n", ( status == GL_TRUE ) ? "was successful" : "FAILED" );

	if ( status == GL_FALSE )
	{
		int info_length = 0;
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &info_length );
		std::vector<char> info_log( info_length );
		glGetShaderInfoLog( shader, info_length, &info_length, info_log.data() );

		printf( "Error log: %s\n", info_log.data() );
	}

	return status;
}

void SetInt( const GLuint program, GLint value, const char * int_name )
{
	const GLint location = glGetUniformLocation( program, int_name );

	if ( location == -1 )
	{
		printf( "Integer value '%s' not found in active shader.\n", int_name );
	}
	else
	{
		glUniform1i( location, value );
	}
}

void SetSampler( const GLuint program, GLenum texture_unit, const char * sampler_name )
{
	const GLint location = glGetUniformLocation( program, sampler_name );

	if ( location == -1 )
	{
		printf( "Texture sampler '%s' not found in active shader.\n", sampler_name );
	}
	else
	{
		glUniform1i( location, texture_unit );
	}
}

void SetMatrix4x4( const GLuint program, const GLfloat * data, const char * matrix_name )
{
	const GLint location = glGetUniformLocation( program, matrix_name );

	if ( location == -1 )
	{
		printf( "Matrix '%s' not found in active shader.\n", matrix_name );
	}
	else
	{
		glUniformMatrix4fv( location, 1, GL_FALSE, data );
	}
}
void SetMatrix3x3(const GLuint program, const GLfloat* data, const char* matrix_name)
{
	const GLint location = glGetUniformLocation(program, matrix_name);

	if (location == -1)
	{
		printf("Matrix '%s' not found in active shader.\n", matrix_name);
	}
	else
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, data);
	}
}

void SetVector3( const GLuint program, const GLfloat * data, const char * vector_name )
{
	const GLint location = glGetUniformLocation( program, vector_name );

	if ( location == -1 )
	{
		printf( "Vector '%s' not found in active shader.\n", vector_name );
	}
	else
	{
		glUniform3fv( location, 1, data );
	}
}

void SetVector2( const GLuint program, const GLfloat * data, const char * vector_name )
{
	const GLint location = glGetUniformLocation( program, vector_name );

	if ( location == -1 )
	{
		printf( "Vector '%s' not found in active shader.\n", vector_name );
	}
	else
	{
		glUniform2fv( location, 1, data );
	}
}
void SetFloat(const GLuint program, const GLfloat value, const char* float_name)
{
	const GLint location = glGetUniformLocation(program, float_name);

	if (location == -1)
	{
		printf("Float value '%s' not found in active shader.\n", float_name);
	}
	else
	{
		glUniform1f(location, value);
	}
}