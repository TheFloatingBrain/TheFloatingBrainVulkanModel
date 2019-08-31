#include <iostream>
#include <vector>
#include <fstream>
#include <set>
#include <cstdlib>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <regex>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#include <stdio.h>

const unsigned short NVDIA_VENDOR_ID_CONSTANT = 4318;

#define NDEBUG

#ifdef NDEBUG
	bool debug = true;
#else
	bool debug = false;
#endif

#ifdef WIN32
	#define PLATFORM_SURFACE_EXTENSION_NAME_MACRO "VK_KHR_win32_surface"
#endif
//To be continued...//

/***************************************
* TODO: Find run time friendly way of **
* supporting extensions in the future! *
* Requires major overhaul! *************
***************************************/

const size_t AMOUNT_OF_SHADER_TYPES_CONSTANT = 9;

enum ShaderType
{
	VERTEX_SHADER_TYPE_ENUMURATION = 0,
	TESSELLATION_CONTROL_SHADER_TYPE_ENUMURATION = 2,
	TESSELLATION_EVALUATION_SHADER_TYPE_ENUMURATION = 3,
	GEOMETRY_SHADER_TYPE_ENUMURATION = 4,
	FRAGMENT_SHADER_TYPE_ENUMURATION = 1,
	COMPUTE_SHADER_TYPE_ENUMURATION = 5,
	OTHER_SHADER_TYPE_ENUMURATION = 6,
	ALL_SHADER_TYPE_ENUMURATION = 7, 
	ALL_GRAPHICS_SHADER_TYPE_ENUMURATION = 8, 
	NONE_SHADER_TYPE_ENUMERATION = ( -1 )
};

const VkShaderStageFlagBits ShaderTypeToShaderStageFlagBit( const ShaderType& shaderType );

const std::string ShaderTypeToString( const ShaderType& shaderType );

enum class ReadShaderMethod : unsigned short int
{
	NON_EXTENSION = 0, 
	EXTENSION = 1, 
	ENUMERATED = 2
};

std::string StripFilePath( std::string file );

ShaderType ReadSPIRVShaderTypeFromFileName( std::string fileName, ReadShaderMethod howToRead, bool fullNames = true );

template< ReadShaderMethod HOW_TO_READ_CONSTANT >
struct ReadShaderType
{
	const ShaderType type;
	ReadShaderType( ShaderType type_ ) :
		type( type_ ) {
	}
	operator ShaderType() const {
		return type;
	}
};

template<>
struct ReadShaderType< ReadShaderMethod::NON_EXTENSION >
{
	const ShaderType type;
	ReadShaderType( std::string fileName, bool fullNames = true ) : 
			type( ReadSPIRVShaderTypeFromFileName( fileName,
					ReadShaderMethod::NON_EXTENSION, fullNames ) ) {
	}
	constexpr operator ShaderType() {
		return type;
	}
};

template<>
struct ReadShaderType< ReadShaderMethod::EXTENSION >
{
	const ShaderType type;
	ReadShaderType( std::string fileName, bool fullNames = true ) :
		type( ReadSPIRVShaderTypeFromFileName( fileName,
			ReadShaderMethod::EXTENSION, fullNames ) ) {
	}
	constexpr operator ShaderType() {
		return type;
	}
};

const char* SHADER_TYPE_STRINGS_CONSTANT[ AMOUNT_OF_SHADER_TYPES_CONSTANT ] = { 
		"vertex", 
		"tesseleation control", 
		"tesselation evaluation", 
		"geometry",
		"fragment",
		"compute", 
		"all", 
		"all graphics",
		"other"
};

const char* SHADER_TYPE_SHORT_STRINGS_CONSTANT[ AMOUNT_OF_SHADER_TYPES_CONSTANT ] = { 
		"vert", 
		"tesc", 
		"tese", 
		"geom", 
		"frag",
		"comp",
		"all", 
		"allg",
		"other"
};

using SPIRV_SOURCE_TYPE = std::vector< char >;

SPIRV_SOURCE_TYPE ReadSPIRVFile( std::string filePath );

struct ShaderSourceData
{
	char* name;
	char* sourcePath;
	ShaderType shaderType;
	SPIRV_SOURCE_TYPE shaderSource;
};

using SHADER_SOURCES_TYPE = std::vector< ShaderSourceData >;

template< ReadShaderMethod HOW_TO_READ_CONSTANT, bool provideSourceCode = false >
struct CreateShaderSourceData
{
	ShaderSourceData sourceData;
	CreateShaderSourceData( ShaderType enumuratedType, const char* filePath, const char* shaderName = nullptr ) : 
			sourceData{ shaderName, filePath, enumuratedType, ReadSPIRVFile( filePath ) } {
	}
	constexpr operator ShaderSourceData() {
		return sourceData;
	}
};

template<>
struct CreateShaderSourceData< ReadShaderMethod::NON_EXTENSION, false >
{
	ShaderSourceData sourceData;
	CreateShaderSourceData( const char* filePath, const char* shaderName = nullptr ) : 
		sourceData{ ( char* ) shaderName, ( char* ) filePath, 
		ReadShaderType< ReadShaderMethod::NON_EXTENSION >( filePath ), ReadSPIRVFile( filePath ) } {
	}
	operator ShaderSourceData() const {
		return sourceData;
	}
};

template<>
struct CreateShaderSourceData< ReadShaderMethod::EXTENSION, false >
{
	ShaderSourceData sourceData;
	CreateShaderSourceData( const char* filePath, const char* shaderName = nullptr ) : 
		sourceData{ ( char* ) shaderName, ( char* ) filePath,
		ReadShaderType< ReadShaderMethod::EXTENSION >( filePath ), ReadSPIRVFile( filePath ) } {
	}
	operator ShaderSourceData() const {
		return sourceData;
	}
};

template<>
struct CreateShaderSourceData< ReadShaderMethod::ENUMERATED, true >
{
	ShaderSourceData sourceData;
	CreateShaderSourceData( ShaderType enumuratedType, 
			SPIRV_SOURCE_TYPE sourceCode, const char* shaderName = nullptr )
	{
		sourceData.name = ( char* ) shaderName;
		sourceData.sourcePath = nullptr;
		sourceData.shaderType = ReadShaderType< ReadShaderMethod::ENUMERATED >( enumuratedType );
	}
	operator ShaderSourceData() const {
		return sourceData;
	}
};

template<>
struct CreateShaderSourceData< ReadShaderMethod::NON_EXTENSION, true > {
	ShaderSourceData sourceData;
	CreateShaderSourceData() = delete;
};

template<>
struct CreateShaderSourceData< ReadShaderMethod::EXTENSION, true > {
	ShaderSourceData sourceData;
	CreateShaderSourceData() = delete;
};


struct ShaderModule
{
	ShaderSourceData sourceData;
	VkShaderModule shaderModule;
	char* entryPoint;
};

const char* DEFAULT_ENTRY_POINT_NAME_CONSTANT = "main";

enum Status {
	SUCCESS_ENUMURATION = 0, 
	FAILURE_ENUMURATION = 1
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack( VkDebugUtilsMessageSeverityFlagBitsEXT messageSverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callBackData,
		void* userData ) {
	std::cerr << "Message::Vulkan::ValidationLayer: " << callBackData->pMessage << "\n";
	return VK_FALSE;
}


struct Surface
{
	VkSurfaceKHR surface;
	GLFWwindow* window = nullptr;
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector< VkSurfaceFormatKHR > formats;
	std::vector< VkPresentModeKHR > presentModes;
};

struct SwapChain
{
	VkSwapchainKHR swapChain;
	VkSurfaceFormatKHR format;
	VkPresentModeKHR presentMode;
	VkExtent2D extent;
	VkFormat imageFormat;
	std::vector< VkImage > images;	
	std::vector< VkImageView > imageViews;
};

struct LogicalDevice
{
	VkDevice logicalDevice;
	uint32_t queue, graphicsFamily;
	VkPhysicalDevice physicalDevice;
	VkBool32 presentSuccess;
	VkQueue que = VK_NULL_HANDLE;
	SwapChain swapChain;
	VkExtent2D extent;
	std::vector< ShaderModule > shaderModules;
};

struct Instance
{
	VkInstance instance;
	std::vector< std::unique_ptr< Surface > > surfaces;
	std::vector< VkPhysicalDevice > physicalDevices;
	std::vector< LogicalDevice > logicalDevices;
};


struct WindowInformation {
	std::string title;
	unsigned int width, height;
};

struct Application
{
 	explicit Application( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate, 
			size_t amountOfWindowsToCreate, std::vector< SHADER_SOURCES_TYPE > shaders, std::vector< char** > entryPoints = {}  );
	explicit Application( std::string title, unsigned int width, unsigned int height, SHADER_SOURCES_TYPE shaders, char** entryPoints = nullptr );
	void Initialize( size_t amountOfInstancesToCreate, std::vector< SHADER_SOURCES_TYPE > shaders, WindowInformation* windowsToCreate = nullptr, 
			size_t amountOfWindowsToCreate = 0, std::vector< char** > entryPoints = {} );
	GLFWwindow* InitializeGLFW( unsigned int width, unsigned int height, std::string title );
	void InitializeVulkan( std::string name, const Instance& instance, SHADER_SOURCES_TYPE shaders, 
			unsigned int width_ = 0, unsigned int height_ = 0, char** entryPoints = nullptr );
	void CreateInstance( std::string name, const VkInstance& instance );
	void GrabPhysicalDevices( const VkInstance& toGrabFrom );
	size_t CreateLogicalDevice( const Instance& instance, const VkPhysicalDevice& physicalDevice );
	void PrintAvailibleExtensions( const VkInstance& instance );
	VkResult InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers, const VkInstance& instance );
	SwapChain& CreateSwapchain( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface );
	std::vector< VkSurfaceFormatKHR > FindDesiredFormats( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface );
	std::vector< VkPresentModeKHR > FindDesiredPresentModes( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface );
	void MakeExtent( LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface );
	std::vector< VkImage >& GetSwapchainImages( LogicalDevice& logicalDevice );
	void MakeSwapChainImageViews( LogicalDevice& logicalDevice, SwapChain& swapChain );
	ShaderModule CreateShaderModule( LogicalDevice& logicalDevice, const ShaderSourceData& source );
	//NOTE: Pipeline is created IN ORDER OF SHADER SOURCES SPECIFIED!//
	void CreateShaderPipelineStages( LogicalDevice& logicalDevice, char** entryPoints = nullptr );
	bool Update();
	bool GLFWUpdate();
	void Destroy();
	void DestroyInstance( const VkInstance& instance );
	protected:
		std::vector< Instance > instances;
		VkApplicationInfo applicationInfo = {};
		const char** glfwExtensions;
		uint32_t glfwExtensionCount = 0;
		VkDebugUtilsMessengerEXT debugMessenger;
		PFN_vkCreateDebugUtilsMessengerEXT createFunction;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction;
		std::vector< VkPhysicalDevice > allPhysicalDevices;
		float queuePriority = 1.0f;
};

int main( int argc, char** args )
{
	;
	Application application{ "Vulkan Example", 800, 600, SHADER_SOURCES_TYPE{ 
			CreateShaderSourceData< ReadShaderMethod::NON_EXTENSION >(
			"Shaders/SPIRV/vert.spv", "HelloPrismVertexShader" ), 
			CreateShaderSourceData< ReadShaderMethod::NON_EXTENSION >(
			"Shaders/SPIRV/frag.spv", "HelloPrismFragmentShader" ) } 
			};
	while( application.Update() );
	application.Destroy();
	return 0;
}

const std::string DEFAULT_WINDOW_TITLE_CONSTANT = "Vulkan Surface Window";

const VkShaderStageFlagBits ShaderTypeToShaderStageFlagBit( 
		const ShaderType& shaderType )
{
	switch( shaderType )
	{
		case VERTEX_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		case TESSELLATION_CONTROL_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case TESSELLATION_EVALUATION_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case GEOMETRY_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		case FRAGMENT_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		case COMPUTE_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		case OTHER_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		case ALL_SHADER_TYPE_ENUMURATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		case ALL_GRAPHICS_SHADER_TYPE_ENUMURATION: 
			return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS;
		case NONE_SHADER_TYPE_ENUMERATION:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		default:
			return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
	}
}

const std::string ShaderTypeToString( const ShaderType& shaderType )
{
	switch( shaderType )
	{
		case VERTEX_SHADER_TYPE_ENUMURATION:
			return "VERTEX_SHADER_TYPE_ENUMURATION";
		case TESSELLATION_CONTROL_SHADER_TYPE_ENUMURATION:
			return "TESSELLATION_CONTROL_SHADER_TYPE_ENUMURATION";
		case TESSELLATION_EVALUATION_SHADER_TYPE_ENUMURATION:
			return "TESSELLATION_EVALUATION_SHADER_TYPE_ENUMURATION";
		case GEOMETRY_SHADER_TYPE_ENUMURATION:
			return "GEOMETRY_SHADER_TYPE_ENUMURATION";
		case FRAGMENT_SHADER_TYPE_ENUMURATION:
			return "FRAGMENT_SHADER_TYPE_ENUMURATION";
		case COMPUTE_SHADER_TYPE_ENUMURATION:
			return "COMPUTE_SHADER_TYPE_ENUMURATION";
		case OTHER_SHADER_TYPE_ENUMURATION:
			return "OTHER_SHADER_TYPE_ENUMURATION";
		case ALL_SHADER_TYPE_ENUMURATION:
			return "ALL_SHADER_TYPE_ENUMURATION";
		case ALL_GRAPHICS_SHADER_TYPE_ENUMURATION: 
			return "ALL_GRAPHICS_SHADER_TYPE_ENUMURATION";
		case NONE_SHADER_TYPE_ENUMERATION:
			return "NONE_SHADER_TYPE_ENUMERATION";
		default:
			return "VALUE_UNKNOWN";
	}
}

std::string StripFilePath( std::string file )
{
	std::string result = file;
	const auto LAST_BACK_SLASH_CONSTANT = file.rfind( "\\" );
	const auto LAST_FORWARD_SLASH_CONSTANT = file.rfind( "/" );
	if( LAST_BACK_SLASH_CONSTANT == std::string::npos && 
			LAST_FORWARD_SLASH_CONSTANT != std::string::npos && 
			( LAST_FORWARD_SLASH_CONSTANT + 1 ) < file.size() )
				result = file.substr( LAST_FORWARD_SLASH_CONSTANT + 1 );
	else if( LAST_FORWARD_SLASH_CONSTANT == std::string::npos && 
			( LAST_BACK_SLASH_CONSTANT + 1 ) < file.size() )
			result = file.substr( LAST_BACK_SLASH_CONSTANT + 1 );
	else
	{
		const size_t LARGER_INDEX_CONSTANT = 
				LAST_BACK_SLASH_CONSTANT > LAST_FORWARD_SLASH_CONSTANT ? 
			LAST_BACK_SLASH_CONSTANT : LAST_FORWARD_SLASH_CONSTANT;
		if( ( LARGER_INDEX_CONSTANT + 1 ) < file.size() ) {
			result = file.substr( LARGER_INDEX_CONSTANT + 1 );
		}
	}
	return result;
}

ShaderType ReadSPIRVShaderTypeFromFileName( std::string fileName, ReadShaderMethod howToRead, bool fullNames )
{
	constexpr const char MARKER_CONSTANT = '%';
	ShaderType result = NONE_SHADER_TYPE_ENUMERATION;
	std::string copy, nameRegex;
	fileName = StripFilePath( fileName );
	if( howToRead == ReadShaderMethod::NON_EXTENSION )
	{
		//OLD: (((([^\w\d\s])(%)([^\w\d\s]))|(([^\w\d\s])(%)$)|(^%$)))

		//(((([^\w\d\s])(%)([^\w\d\s]))|(([^\w\d\s])(%)$)|(^%$)|((^%)([^\w\d\s]))))
		nameRegex = std::string{ std::string{ "(((([^\\w\\d\\s])(" } +
				MARKER_CONSTANT + std::string{ ")([^\\w\\d\\s]))|(([^\\w\\d\\s])(" } +
				MARKER_CONSTANT + std::string{ ")$)|(^" } + 
				MARKER_CONSTANT + std::string{ "$)|((^" } + 
				MARKER_CONSTANT + std::string{ ")([^\\w\\d\\s]))))" }
				};
	}
	else if( howToRead == ReadShaderMethod::EXTENSION )
	{
		//(([.])(%)([.]))|(([.])(%)$)
		nameRegex = std::string{ std::string{ "(([.])(" } +
				MARKER_CONSTANT + std::string{ ")([.]))|(([.])(" } +
				MARKER_CONSTANT + std::string{ ")$)" }
				};
	}
	const size_t NAME_REGEX_SIZE_CONSTANT = nameRegex.size();
	std::vector< size_t > markerPositions;
	const char** shaderTypeStringSets[ 2 ] = {
		SHADER_TYPE_SHORT_STRINGS_CONSTANT,
		SHADER_TYPE_STRINGS_CONSTANT
	};
	for( size_t i = 0; i < NAME_REGEX_SIZE_CONSTANT; ++i )
	{
		const size_t POSITION_CONSTANT = nameRegex.find( MARKER_CONSTANT, i );
		if( POSITION_CONSTANT != std::string::npos )
		{
			markerPositions.push_back( POSITION_CONSTANT );
			//One will be added to i.//
			i = POSITION_CONSTANT;
			continue;
		}
		else
			break;
	}
	const size_t AMOUNT_OF_MARKER_POSITIONS_CONSTANT = markerPositions.size();
	for( unsigned short int currentSet = 0; currentSet < 2; ++currentSet )
	{
		for( size_t i = 0; i < AMOUNT_OF_SHADER_TYPES_CONSTANT; ++i )
		{
			const size_t NAME_SIZE_CONSTANT = strlen( shaderTypeStringSets[ currentSet ][ i ] );
			copy = nameRegex;
			for( size_t j = 0; j < AMOUNT_OF_MARKER_POSITIONS_CONSTANT; ++j )
			{
				copy.erase( copy.begin() + markerPositions[ j ] + ( j * NAME_SIZE_CONSTANT ) - j );
				copy.insert( markerPositions[ j ] + ( j * NAME_SIZE_CONSTANT ) - j,
					shaderTypeStringSets[ currentSet ][ i ] );
			}
			if( std::regex_search( fileName, std::regex{ copy, std::regex_constants::icase } ) == true )
			{
				result = ( ShaderType ) i;
				currentSet = 2;
				break;
			}
		}
	}
	return result;
}

SPIRV_SOURCE_TYPE ReadSPIRVFile( std::string filePath )
{
	std::ifstream file{ filePath, std::ios::ate | std::ios::binary };
	SPIRV_SOURCE_TYPE source;
	if( file.is_open() )
	{
		const size_t FILE_SIZE_CONSTANT = ( size_t ) file.tellg();
		source.resize( FILE_SIZE_CONSTANT );
		file.seekg( 0 );
		file.read( source.data(), FILE_SIZE_CONSTANT );
		return source;
	}
	else
		std::cerr << "Error::ReadSPIRVFile( std::string ): SPIRV_SOURCE_TYPE: Failed to open file.\n";
	file.close();
	return source;
}

Application::Application( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate, size_t amountOfWindowsToCreate, 
		std::vector< SHADER_SOURCES_TYPE > shaders, std::vector< char** > entryPoints ) {
	Initialize( amountOfInstancesToCreate, shaders, windowsToCreate, amountOfWindowsToCreate, entryPoints );
}

Application::Application( std::string title, unsigned int width, unsigned int height, 
		SHADER_SOURCES_TYPE shaders, char** entryPoints ) {
	WindowInformation windowInformation{ title, width, height };
	Initialize( 1, std::vector< SHADER_SOURCES_TYPE >{ shaders }, &windowInformation, 1, { entryPoints } );
}

void Application::Initialize( size_t amountOfInstancesToCreate, std::vector< SHADER_SOURCES_TYPE > shaders, 
		WindowInformation* windowsToCreate, size_t amountOfWindowsToCreate, std::vector< char** > entryPoints )
{
	instances.resize( amountOfInstancesToCreate );
	const size_t AMOUNT_OF_ENTRY_POINTS_CONSTANT = entryPoints.size();
	char** entryPointsForThisInstance = nullptr;
	for( unsigned int i = 0; i < amountOfInstancesToCreate; ++i )
	{
		if( i < amountOfWindowsToCreate )
		{
			if( AMOUNT_OF_ENTRY_POINTS_CONSTANT > 1 )
				entryPointsForThisInstance = entryPoints[ i ];
			InitializeVulkan( windowsToCreate[ i ].title,
				instances[ i ], shaders[ i ], windowsToCreate[ i ].width, 
				windowsToCreate[ i ].height, entryPointsForThisInstance );
		}
		else {
			InitializeVulkan( DEFAULT_WINDOW_TITLE_CONSTANT, instances[ i ], shaders[ 0 ],
					windowsToCreate[ 0 ].width, windowsToCreate[ 0 ].height, entryPoints[ 0 ] );
		}
	}
}

GLFWwindow* Application::InitializeGLFW( unsigned int width, unsigned int height, std::string title )
{
	glfwInit();
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
	return glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
}

void Application::PrintAvailibleExtensions( const VkInstance& instance )
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );
	std::vector< VkExtensionProperties > extensions( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );
	std::cout << "Available Extensions: \n";
	for(const auto& currentExtension : extensions)
		std::cout << "\t" << currentExtension.extensionName << "\n";
}

void Application::InitializeVulkan( std::string name, const Instance& instance, SHADER_SOURCES_TYPE shaders, 
		unsigned int width, unsigned int height, char** entryPoints )
{
	CreateInstance( name, instance.instance );
	if( allPhysicalDevices.size() == 0 )
		GrabPhysicalDevices( instance.instance );
	VkPhysicalDevice physicalDevice = allPhysicalDevices[ allPhysicalDevices.size() - 1 ];
	allPhysicalDevices.pop_back();
	( ( Instance& ) instance ).physicalDevices.push_back( physicalDevice );
	if( width != 0 || height != 0 )
	{
		( ( Instance& ) instance ).surfaces.push_back( std::make_unique< Surface >() );
		const std::unique_ptr< Surface >& surface = instance.surfaces[ instance.surfaces.size() - 1 ];
		surface->window = InitializeGLFW( width, height, name );
		VkResult result = glfwCreateWindowSurface( instance.instance, surface->window, nullptr, &surface->surface );
		CreateLogicalDevice( instance, physicalDevice );
		if( result == VK_SUCCESS )
		{
			std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Surface creation successful!\n";
			if( glfwVulkanSupported() )
			{
				std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: GLFW Supports Vulkan!\n";
				auto& logicalDevice = instance.logicalDevices[ instance.logicalDevices.size() - 1 ];
				SwapChain& swapChain = CreateSwapchain( logicalDevice, surface );
				for( auto& currentShaderData : shaders )
					CreateShaderModule( ( LogicalDevice& ) logicalDevice, currentShaderData );
				CreateShaderPipelineStages( ( LogicalDevice& ) logicalDevice, entryPoints );
			}
			else
				std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: GLFW Does Not Support Vulkan!\n";
		}
		else
			std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Failed to create surface error: " << result << "\n";
	}
}

void Application::CreateInstance( std::string name, const VkInstance& instance )
{
	VkInstanceCreateInfo createInfo = {};
	std::vector< const char* > validationLayers{ "VK_LAYER_KHRONOS_validation" };
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = name.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	PrintAvailibleExtensions( instance );
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
	std::vector< const char* > enabledExtensions( glfwExtensions, glfwExtensions + glfwExtensionCount );
	createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
	createInfo.ppEnabledLayerNames = validationLayers.data();

	enabledExtensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
	enabledExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
	enabledExtensions.push_back( VK_KHR_SURFACE_EXTENSION_NAME );
	enabledExtensions.push_back( PLATFORM_SURFACE_EXTENSION_NAME_MACRO );
	
	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if( vkCreateInstance( &createInfo, nullptr, &( ( VkInstance& ) instance ) ) != VK_SUCCESS ) {
		std::cerr << "Error::InitializeVulkan( std::string, const VkInstance& instance ):void: Failed to create instance.\n";
		return;
	}
	if( debug == true )
	{
		std::cout << "Note::InitializeVulkan( std::string, const VkInstance& instance ):void: Debug mode, enabling validation layers.\n";
		auto result = InitializeVulkanDebugLayer( std::move( validationLayers ), instance );
		if( result == VK_SUCCESS )
			std::cout << "Note::InitializeVulkan( std::string, const VkInstance& instance ):void: Debug layer creation succesful!\n";
	}
}

void Application::GrabPhysicalDevices( const VkInstance& toGrabFrom )
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	uint32_t amountOfAvailiblePhysicalDevices;
	vkEnumeratePhysicalDevices( toGrabFrom, &amountOfAvailiblePhysicalDevices, nullptr );
	std::cout << "Note::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: Amount of Physical Devices Availible: " << amountOfAvailiblePhysicalDevices << ".\n";
	if( amountOfAvailiblePhysicalDevices == 0 ) {
		std::cerr << "Error::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: No availible physical devices.\n";
		return;
	}
	allPhysicalDevices.clear();
	allPhysicalDevices.resize( amountOfAvailiblePhysicalDevices );
	vkEnumeratePhysicalDevices( toGrabFrom, &amountOfAvailiblePhysicalDevices, allPhysicalDevices.data() );
	std::cout << "Note::GrabPhysicalDevice( const VkInstance& toGrabFrom ):void: Found devices:\n";
	int nvidia = -1;
	const unsigned int AMOUNT_OF_DEVICES_CONSTANT = allPhysicalDevices.size();
	for( unsigned int i = 0; i < AMOUNT_OF_DEVICES_CONSTANT; ++i )
	{
		vkGetPhysicalDeviceProperties( allPhysicalDevices[ i ], &deviceProperties );
		vkGetPhysicalDeviceFeatures( allPhysicalDevices[ i ], &deviceFeatures );
		std::cout << "\t* : Device Name: " << deviceProperties.deviceName << "\n";
		if( deviceProperties.vendorID == NVDIA_VENDOR_ID_CONSTANT ) {
			std::cout << "\t\t--> This is an NVDIA device\n";
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties( allPhysicalDevices[ i ], nullptr, &extensionCount, nullptr );

			std::vector<VkExtensionProperties> availableExtensions( extensionCount );
			vkEnumerateDeviceExtensionProperties( allPhysicalDevices[ i ], nullptr, &extensionCount, availableExtensions.data() );

			bool supportsSwapchain = false;
			for( const auto& extension : availableExtensions )
			{
				if( strcmp( extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME ) == 0 ) {
					supportsSwapchain = true;
					break;
				}
			}
			if( supportsSwapchain == true ) {
				std::cout << "\t\t--> This device supports swapchain.\n";
				nvidia = i;
			}
		}
	}
	if( nvidia != ( -1 ) ) 
	{
		VkPhysicalDevice toSwap = allPhysicalDevices[ AMOUNT_OF_DEVICES_CONSTANT - 1 ];
		allPhysicalDevices[ AMOUNT_OF_DEVICES_CONSTANT - 1 ] = allPhysicalDevices[ nvidia ];
		allPhysicalDevices[ nvidia ] = toSwap;
	}
}

size_t Application::CreateLogicalDevice( const Instance& instance, const VkPhysicalDevice& physicalDevice )
{
	uint32_t queueFamilyCount = 0;
	uint32_t graphicsFamily = 0;
	std::vector< VkQueueFamilyProperties > queueFamilies;
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo logicalDeviceCreationInfo = {};
	VkDevice logicalDevice;
	unsigned int queueFamilyIndex = -1;
	logicalDeviceCreationInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );
	queueFamilies.resize( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data() );
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	const size_t AMOUNT_OF_QUEUE_FAMILIES_CONSTANT = queueFamilies.size();
	std::unique_ptr< Surface >& surface = ( ( Instance& ) instance ).surfaces[ instance.surfaces.size() - 1 ];
	std::cout << "Note::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Amount of queue families is " << AMOUNT_OF_QUEUE_FAMILIES_CONSTANT << ".\n";
	VkBool32 presentSupport = VK_FALSE;
	bool foundPresent = false;
	for( unsigned int i = 0; i < AMOUNT_OF_QUEUE_FAMILIES_CONSTANT; ++i )
	{
		if( queueFamilies[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
			graphicsFamily = i;
		if( foundPresent == false )
		{
			vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, surface->surface, &presentSupport );
			if( presentSupport == VK_TRUE ) {
				queueFamilyIndex = i;
				foundPresent = true;
			}
		}
	}
	if( queueFamilyIndex == -1 )
		std::cerr << "Error::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Failed to find suitable queue family.\n";
	queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	logicalDeviceCreationInfo.pQueueCreateInfos = &queueCreateInfo;
	logicalDeviceCreationInfo.queueCreateInfoCount = 1;
	logicalDeviceCreationInfo.pEnabledFeatures = &deviceFeatures;
	logicalDeviceCreationInfo.enabledExtensionCount = 1;
	std::vector< const char* > enabledExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	logicalDeviceCreationInfo.ppEnabledExtensionNames = enabledExtensions.data();
	if( vkCreateDevice( physicalDevice, &logicalDeviceCreationInfo, nullptr, &logicalDevice ) != VK_SUCCESS ) {
		std::cerr << "Error::CreateLogicalDevice( const VkInstance&, const VkPhysicalDevice& ):void: Failed to create logic device.\n";
		logicalDevice = VK_NULL_HANDLE;
	}
	( ( Instance& ) instance ).logicalDevices.push_back( LogicalDevice{ logicalDevice, queueFamilyIndex, graphicsFamily, physicalDevice, presentSupport } );
	return ( instance.logicalDevices.size() - 1 );
}

VkResult Application::InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers, const VkInstance& instance )
{
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
	debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugMessengerCreateInfo.pfnUserCallback = DebugCallBack;
	debugMessengerCreateInfo.pUserData = nullptr;

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );
	std::vector< VkLayerProperties > availableLayers( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );
	std::cout << "Note::InitializeVulkanDebugLayer():VkResult: Availible layer count is " << layerCount << ".\n";

	for( const char* layerName : validationLayers )
	{
		bool layerFound = false;
		for( const auto& layerProperties : availableLayers )
		{
			if( strcmp( layerName, layerProperties.layerName ) == 0 )
			{
				layerFound = true;
				std::cout << "Note::InitializeVulkanDebugLayer():VkResult: Comfirmed layer " << layerProperties.layerName << ".\n";
				break;
			}
		}

		if( !layerFound ) {
			std::cerr << "Error::InitializeVulkanDebugLayer():VkResult: Failed to confirm all layer names.\n";
			return VkResult::VK_INCOMPLETE;
		}
	}


	createFunction = ( PFN_vkCreateDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );

	destroyFunction = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );

	if( createFunction != nullptr )
		return createFunction( instance, &debugMessengerCreateInfo, nullptr, &debugMessenger );
	else {
		std::cerr << "Error::InitializeVulkanDebugLayer():VkResult: vkCreateDebugUtilsMessengerEXT function is null.\n";
	return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

std::vector< VkSurfaceFormatKHR > Application::FindDesiredFormats( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface )
{
	std::vector< VkSurfaceFormatKHR > desiredFormats;
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( logicalDevice.physicalDevice, surface->surface, &formatCount, nullptr );
	if( formatCount != 0 ) {
		surface->formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( logicalDevice.physicalDevice, surface->surface, &formatCount, surface->formats.data() );
	}
	for( auto& format : surface->formats )
	{
		if( format.format == VK_FORMAT_B8G8R8A8_UNORM )
		{
			if( format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
			{
				if( desiredFormats.size() == 0 )
					desiredFormats.push_back( format );
				else
				{
					VkSurfaceFormatKHR toSwap = desiredFormats[ 0 ];
					desiredFormats.push_back( toSwap );
					desiredFormats[ 0 ] = format;
				}
			}
			else
				desiredFormats.push_back( format );
		}
	}
	return desiredFormats;
}

std::vector< VkPresentModeKHR > Application::FindDesiredPresentModes( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface )
{
	std::vector< VkPresentModeKHR > desiredPresentModes;
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( logicalDevice.physicalDevice, surface->surface, &presentModeCount, nullptr );
	if( presentModeCount != 0 ) {
		surface->presentModes.resize( presentModeCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( logicalDevice.physicalDevice, surface->surface, &presentModeCount, surface->presentModes.data() );
	}
	bool hasFIFO = false;
	for( const auto& mode : surface->presentModes )
	{
		if( mode == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			if( desiredPresentModes.size() == 0 )
				desiredPresentModes.push_back( mode );
			else
				desiredPresentModes.insert( desiredPresentModes.begin(), mode );
		}
		else if( mode == VK_PRESENT_MODE_IMMEDIATE_KHR )
			desiredPresentModes.push_back( mode );
		else if( mode == VK_PRESENT_MODE_FIFO_KHR )
			hasFIFO = true;
	}
	if( hasFIFO == true )
		desiredPresentModes.push_back( VK_PRESENT_MODE_FIFO_KHR );
	return desiredPresentModes;
}

void Application::MakeExtent( LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface )
{
	if( surface->capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
		logicalDevice.swapChain.extent = surface->capabilities.currentExtent;
	else
	{
		int width, height;
		glfwGetWindowSize( surface->window, &width, &height );
		logicalDevice.swapChain.extent  = { ( uint32_t ) width, ( uint32_t ) height };
		logicalDevice.swapChain.extent.width = std::max( surface->capabilities.minImageExtent.width,
			std::min( surface->capabilities.maxImageExtent.width, logicalDevice.swapChain.extent.width ) );
		logicalDevice.swapChain.extent.height = std::max( surface->capabilities.minImageExtent.height,
			std::min( surface->capabilities.maxImageExtent.height, logicalDevice.swapChain.extent.height ) );
		logicalDevice.extent = logicalDevice.swapChain.extent;
	}
}

std::vector< VkImage >& Application::GetSwapchainImages( LogicalDevice& logicalDevice )
{
	size_t swapChainImageCount;
	vkGetSwapchainImagesKHR( logicalDevice.logicalDevice, logicalDevice.swapChain.swapChain, &swapChainImageCount, nullptr );
	( ( LogicalDevice& ) logicalDevice ).swapChain.images.resize( swapChainImageCount );
	vkGetSwapchainImagesKHR( logicalDevice.logicalDevice, logicalDevice.swapChain.swapChain,
		&swapChainImageCount, ( ( LogicalDevice& ) logicalDevice ).swapChain.images.data() );
	logicalDevice.swapChain.imageFormat = logicalDevice.swapChain.format.format;
	return ( ( LogicalDevice& ) logicalDevice ).swapChain.images;
}

void Application::MakeSwapChainImageViews( LogicalDevice& logicalDevice, SwapChain& swapChain )
{
	const size_t AMOUNT_OF_IMAGE_VIEWS_CONSTANT = swapChain.images.size();
	swapChain.imageViews.resize( AMOUNT_OF_IMAGE_VIEWS_CONSTANT );
	for( size_t i = 0; i < AMOUNT_OF_IMAGE_VIEWS_CONSTANT; ++i )
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChain.images[ i ];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChain.imageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		if( vkCreateImageView( logicalDevice.logicalDevice, &createInfo, nullptr, &swapChain.imageViews[ i ] ) == VK_SUCCESS )
			std::cout << "Note::MakeSwapChainImageViews( LogicalDevice&, SwapChain& ):void: Successfully created image view " << i << ".\n";
		else
			std::cerr << "Error::MakeSwapChainImageViews( LogicalDevice&, SwapChain& ):void: Failed to create image view " << i << ".\n";
	}
}

SwapChain& Application::CreateSwapchain( const LogicalDevice& logicalDevice, const std::unique_ptr< Surface >& surface )
{
	if( logicalDevice.presentSuccess == VK_TRUE )
	{
		std::cout << "Note::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: This device has present support.\n";
		vkGetDeviceQueue( logicalDevice.logicalDevice, logicalDevice.queue, 0, ( VkQueue* ) &logicalDevice.que );
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR( logicalDevice.physicalDevice, surface->surface, &surface->capabilities );
		std::vector< VkSurfaceFormatKHR > desiredFormats = FindDesiredFormats( logicalDevice, surface );
		std::vector< VkPresentModeKHR > desiredPresentModes = FindDesiredPresentModes( logicalDevice, surface );
		if( !surface->formats.empty() && !surface->presentModes.empty() )
		{
			std::cout << "Note::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: Swap chain format and presentation mode supported!\n";
			MakeExtent( ( LogicalDevice& ) logicalDevice, surface );
			if( desiredFormats.size() != 0 )
			{
				uint32_t imageCount = surface->capabilities.minImageCount + 1;
				if( surface->capabilities.maxImageCount > 0 && imageCount < surface->capabilities.maxImageCount )
					imageCount = surface->capabilities.maxImageCount;
				( ( LogicalDevice& ) logicalDevice ).swapChain.format = desiredFormats[ 0 ];
				( ( LogicalDevice& ) logicalDevice ).swapChain.presentMode = desiredPresentModes[ 0 ];
				VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
				swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapChainCreateInfo.surface = surface->surface;
				swapChainCreateInfo.minImageCount = imageCount;
				swapChainCreateInfo.imageFormat = logicalDevice.swapChain.format.format;
				swapChainCreateInfo.imageColorSpace = logicalDevice.swapChain.format.colorSpace;
				swapChainCreateInfo.imageExtent = logicalDevice.swapChain.extent;
				swapChainCreateInfo.imageArrayLayers = 1;
				swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				if( logicalDevice.graphicsFamily != logicalDevice.queue )
				{
					uint32_t queueIndicies[] = { logicalDevice.graphicsFamily, logicalDevice.queue };
					swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
					swapChainCreateInfo.queueFamilyIndexCount = 2;
					swapChainCreateInfo.pQueueFamilyIndices = queueIndicies;
				}
				else
				{
					swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
					swapChainCreateInfo.queueFamilyIndexCount = 0;
					swapChainCreateInfo.pQueueFamilyIndices = nullptr;
				}
				swapChainCreateInfo.preTransform = surface->capabilities.currentTransform;
				//TODO: CHANGE LATER.//
				swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapChainCreateInfo.presentMode = logicalDevice.swapChain.presentMode;
				swapChainCreateInfo.clipped = VK_TRUE;
				//TODO: CHANGE LATER.//
				swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
				if( vkCreateSwapchainKHR( logicalDevice.logicalDevice, &swapChainCreateInfo, nullptr, ( VkSwapchainKHR* ) &logicalDevice.swapChain.swapChain ) == VK_SUCCESS )
				{
					GetSwapchainImages( ( LogicalDevice& ) logicalDevice );
					MakeSwapChainImageViews( ( LogicalDevice& ) logicalDevice, ( ( LogicalDevice& ) logicalDevice ).swapChain );
					std::cout << "Note::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: Successfully created swap chain!\n";
				}
				else
					std::cerr << "Error::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: Failed to create swap chain!\n";
			}
			else
				std::cerr << "Error::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: No desired formats!\n";
		}
		else
			std::cerr << "Error::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: Swap chain format and presentation mode NOT supported!\n";
	}
	else
		std::cerr << "Error::CreateSwapchain( const LogicalDevice&, const std::unique_ptr< Surface >& ):SwapChain&: This device does not have present support.\n";
	return ( ( LogicalDevice& ) logicalDevice ).swapChain;
}

ShaderModule Application::CreateShaderModule( LogicalDevice& logicalDevice, const ShaderSourceData& source )
{
	VkShaderModuleCreateInfo shaderModuleCreationInformation = {};
	ShaderModule shaderModule;
	shaderModule.sourceData = source;
	shaderModuleCreationInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreationInformation.codeSize = source.shaderSource.size();
	shaderModuleCreationInformation.pCode = reinterpret_cast< const uint32_t* >( source.shaderSource.data() );
	if( vkCreateShaderModule( logicalDevice.logicalDevice, &shaderModuleCreationInformation, nullptr, &shaderModule.shaderModule ) == VK_SUCCESS )
	{
		logicalDevice.shaderModules.push_back( shaderModule );
		std::cout << "Note::Application::CreateShaderModule( LogicalDevice&, const SPIRV_SOURCE_TYPE& ): void: Successfully created shader module!\n";
	}
	else
		std::cerr << "Error::Application::CreateShaderModule( LogicalDevice&, const SPIRV_SOURCE_TYPE& ): void: Failed to create shader module!\n";
	return shaderModule;
}

void Application::CreateShaderPipelineStages( LogicalDevice& logicalDevice, char** entryPoints )
{
	size_t currentModule = 0;
	if( entryPoints == nullptr )
	{
		const size_t AMOUNT_OF_ENTRY_POINTS_CONSTANT = logicalDevice.shaderModules.size();
		entryPoints = new char*[ AMOUNT_OF_ENTRY_POINTS_CONSTANT ];
		for( size_t i = 0; i < AMOUNT_OF_ENTRY_POINTS_CONSTANT; ++i )
			entryPoints[ i ] = ( char* ) DEFAULT_ENTRY_POINT_NAME_CONSTANT;
	}
	for( auto& shaderModule : logicalDevice.shaderModules )
	{
		VkPipelineShaderStageCreateInfo shaderModuleStageCreationInformation = {};
		shaderModuleStageCreationInformation.sType = 
				VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderModuleStageCreationInformation.stage = 
				ShaderTypeToShaderStageFlagBit( 
					shaderModule.sourceData.shaderType );
		shaderModuleStageCreationInformation.module = shaderModule.shaderModule;
		shaderModuleStageCreationInformation.pName = entryPoints[ currentModule++ ];
	}
	std::cout << "Note::Application::CreateShaderPipelineStages( LogicalDevice&, char** ): "
			"void::Successfully created shader pipeline stages!\n";
}

bool Application::Update() {
	return GLFWUpdate();
}

bool Application::GLFWUpdate()
{
	bool run = false;
	for( const Instance& currentInstance : instances )
	{
		for( const std::unique_ptr< Surface >& currentSurface : currentInstance.surfaces )
		{
			if( currentSurface->window != nullptr )
			{
				if( glfwWindowShouldClose( currentSurface->window ) == false ) {
					glfwPollEvents();
					run = true;
				}
			}
		}
	}
	return run;
}

void Application::DestroyInstance( const VkInstance& instance )
{
	if( debug == true )
		destroyFunction( instance, debugMessenger, nullptr );
	vkDestroyInstance( instance, nullptr );
}

void Application::Destroy()
{
	const size_t AMOUNT_OF_INSTANCES_CONSTANT = instances.size();
	for( unsigned int i = 0; i < AMOUNT_OF_INSTANCES_CONSTANT; ++i )
	{
		const size_t AMOUNT_OF_LOGICAL_DEVICES_CONSTANT = instances[ i ].logicalDevices.size();
		for( unsigned int j = 0; j < AMOUNT_OF_LOGICAL_DEVICES_CONSTANT; ++j )
		{
			for( auto& shaderModule : instances[ i ].logicalDevices[ j ].shaderModules )
				vkDestroyShaderModule( instances[ i ].logicalDevices[ j ].
					logicalDevice, shaderModule.shaderModule, nullptr );
			for( auto& imageView : instances[ i ].logicalDevices[ j ].swapChain.imageViews )
				vkDestroyImageView( instances[ i ].logicalDevices[ j ].logicalDevice, imageView, nullptr );
			vkDestroySwapchainKHR( instances[ i ].logicalDevices[ j ].logicalDevice,
					instances[ i ].logicalDevices[ j ].swapChain.swapChain, nullptr );
			vkDestroyDevice( instances[ i ].logicalDevices[ j ].logicalDevice, nullptr );
		}
		const size_t AMOUNT_OF_SURFACES_CONSTANT = instances[ i ].surfaces.size();
		for( unsigned int j = 0; j < AMOUNT_OF_SURFACES_CONSTANT; ++j )
		{
			vkDestroySurfaceKHR( instances[ i ].instance,
				instances[ i ].surfaces[ j ]->surface, nullptr );
			glfwDestroyWindow( instances[ i ].surfaces[ j ]->window );
		}
		DestroyInstance( instances[ i ].instance );
	}
	glfwTerminate();
}
