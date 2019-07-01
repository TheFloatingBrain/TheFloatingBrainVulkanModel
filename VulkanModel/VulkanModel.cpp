#include <iostream>
#include <vector>
#include <set>
#include <cstdlib>
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

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

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallBack( VkDebugUtilsMessageSeverityFlagBitsEXT messageSverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callBackData,
		void* userData ) {
	std::cerr << "Message::Vulkan::ValidationLayer: " << callBackData->pMessage << "\n";
	return VK_FALSE;
}


struct Surface {
	VkSurfaceKHR surface;
	GLFWwindow* window = nullptr;
};

struct LogicalDevice
{
	VkDevice logicalDevice;
	uint32_t queue, graphicsFamily;
	VkPhysicalDevice physicalDevice;
	VkBool32 presentSuccess;
	VkQueue que = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain;
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
 	explicit Application( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate, size_t amountOfWindowsToCreate );
	explicit Application( std::string title, unsigned int width, unsigned int height );
	void Initialize( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate = nullptr, size_t amountOfWindowsToCreate = 0 );
	GLFWwindow* InitializeGLFW( unsigned int width, unsigned int height, std::string title );
	void InitializeVulkan( std::string name, const Instance& instance, unsigned int width_ = 0, unsigned int height_ = 0 );
	void CreateInstance( std::string name, const VkInstance& instance );
	void GrabPhysicalDevices( const VkInstance& toGrabFrom );
	size_t CreateLogicalDevice( const Instance& instance, const VkPhysicalDevice& physicalDevice );
	void PrintAvailibleExtensions( const VkInstance& instance );
	VkResult InitializeVulkanDebugLayer( std::vector< const char* >&& validationLayers, const VkInstance& instance );
	bool Update();
	bool GLFWUpdate();
	void Destroy();
	void DestroyInstance( const VkInstance& instance );
	protected:
		std::vector< Instance > instancesAndSurfaces;
		VkApplicationInfo applicationInfo = {};
		VkInstanceCreateInfo createInfo = {};
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {};
		const char** glfwExtensions;
		uint32_t glfwExtensionCount = 0;
		VkDebugUtilsMessengerEXT debugMessenger;
		PFN_vkCreateDebugUtilsMessengerEXT createFunction;
		PFN_vkDestroyDebugUtilsMessengerEXT destroyFunction;
		std::vector< VkPhysicalDevice > allPhysicalDevices;
		float queuePriority = 1.0f;
};

int main()
{
	Application application{ "Vulkan Example", 800, 600 };
	while( application.Update() );
	application.Destroy();
	return 0;
}
const std::string DEFAULT_WINDOW_TITLE_CONSTANT = "Vulkan Surface Window";

Application::Application( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate, size_t amountOfWindowsToCreate ) {
	Initialize( amountOfInstancesToCreate, windowsToCreate, amountOfWindowsToCreate );
}
Application::Application( std::string title, unsigned int width, unsigned int height ) {
	WindowInformation windowInformation{ title, width, height };
	Initialize( 1, &windowInformation, 1 );
}
void Application::Initialize( size_t amountOfInstancesToCreate, WindowInformation* windowsToCreate, size_t amountOfWindowsToCreate )
{
	instancesAndSurfaces.resize( amountOfInstancesToCreate );
	for( unsigned int i = 0; i < amountOfInstancesToCreate; ++i )
	{
		if( i < amountOfWindowsToCreate ) {
			InitializeVulkan( windowsToCreate[ i ].title,
				instancesAndSurfaces[ i ], windowsToCreate[ i ].width, windowsToCreate[ i ].height );
		}
		else
			InitializeVulkan( DEFAULT_WINDOW_TITLE_CONSTANT, instancesAndSurfaces[ i ] );
	}
}
GLFWwindow* Application::InitializeGLFW(unsigned int width, unsigned int height, std::string title )
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
void Application::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height )
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
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector< VkSurfaceFormatKHR > formats;
				std::vector< VkPresentModeKHR > presentModes;
				std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: GLFW Supports Vulkan!\n";
				auto& logicalDevice = instance.logicalDevices[ instance.logicalDevices.size() - 1 ];
				if( logicalDevice.presentSuccess == VK_TRUE )
				{
					std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: This device has present support.\n";
					vkGetDeviceQueue( logicalDevice.logicalDevice, logicalDevice.queue, 0, ( VkQueue* ) &logicalDevice.que );
					vkGetPhysicalDeviceSurfaceCapabilitiesKHR( logicalDevice.physicalDevice, surface->surface, &capabilities );
					uint32_t formatCount;
					vkGetPhysicalDeviceSurfaceFormatsKHR( logicalDevice.physicalDevice, surface->surface, &formatCount, nullptr );
					if( formatCount != 0 ) {
						formats.resize( formatCount );
						vkGetPhysicalDeviceSurfaceFormatsKHR( logicalDevice.physicalDevice, surface->surface, &formatCount, formats.data() );
					}
					uint32_t presentModeCount;
					vkGetPhysicalDeviceSurfacePresentModesKHR( logicalDevice.physicalDevice, surface->surface, &presentModeCount, nullptr );
					if( presentModeCount != 0 ) {
						presentModes.resize( presentModeCount );
						vkGetPhysicalDeviceSurfacePresentModesKHR( logicalDevice.physicalDevice, surface->surface, &presentModeCount, presentModes.data() );
					}
					if( !formats.empty() && !presentModes.empty() )
					{
						std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Swap chain format and presentation mode supported!\n";
						std::vector< VkSurfaceFormatKHR > desiredFormats;
						std::vector< VkPresentModeKHR > desiredPresentModes;
						for( auto& format : formats )
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
						bool hasFIFO = false;
						for( const auto& mode : presentModes )
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
						/////////////////////////////////////////////////////////////////
						VkExtent2D extent;
						if( capabilities.currentExtent.width != std::numeric_limits< uint32_t >::max() )
							extent = capabilities.currentExtent;
						else
						{
							int width, height;
							glfwGetWindowSize( surface->window, &width, &height );
							extent = { ( uint32_t ) width, ( uint32_t ) height };
							extent.width = std::max( capabilities.minImageExtent.width, 
									std::min( capabilities.maxImageExtent.width, extent.width ) );
							extent.height = std::max( capabilities.minImageExtent.height,
								std::min( capabilities.maxImageExtent.height, extent.height ) );
						}
						/////////////////////////////////////////////////////////////////
						if( desiredFormats.size() != 0 )
						{
							uint32_t imageCount = capabilities.minImageCount + 1;
							if( capabilities.maxImageCount > 0 && imageCount < capabilities.maxImageCount )
								imageCount = capabilities.maxImageCount;
							VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
							swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
							swapChainCreateInfo.surface = surface->surface;
							swapChainCreateInfo.minImageCount = imageCount;
							swapChainCreateInfo.imageFormat = desiredFormats[ 0 ].format;
							swapChainCreateInfo.imageColorSpace = desiredFormats[ 0 ].colorSpace;
							swapChainCreateInfo.imageExtent = extent;
							swapChainCreateInfo.imageArrayLayers = 1;
							swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
							std::cout << "Note::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Swap chain info successfully created!!\n";
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
							swapChainCreateInfo.preTransform = capabilities.currentTransform;
							//TODO: CHANGE LATER.//
							swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
							swapChainCreateInfo.presentMode = presentModes[ 0 ];
							swapChainCreateInfo.clipped = VK_TRUE;
							//TODO: CHANGE LATER.//
							swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
							if( vkCreateSwapchainKHR( logicalDevice.logicalDevice, &swapChainCreateInfo, nullptr, ( VkSwapchainKHR* ) &logicalDevice.swapChain ) )
							{
								std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Failed to create swap chain!\n";
							}
						}
						else
							std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: No desired formats!\n";
					}
					else
						std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: Swap chain format and presentation mode NOT supported!\n";
				}
				else
					std::cerr << "Error::InitializeVulkan( std::string name, const Instance& instance, unsigned int width, unsigned int height ):void: This device does not have present support.\n";
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
	for( unsigned int i = 0; i < AMOUNT_OF_QUEUE_FAMILIES_CONSTANT; ++i )
	{
		if( queueFamilies[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
			graphicsFamily = i;
		vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, i, surface->surface, &presentSupport );
		if( presentSupport == VK_TRUE ) {
			queueFamilyIndex = i;
			presentSupport = VK_FALSE;
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
bool Application::Update() {
	return GLFWUpdate();
}
bool Application::GLFWUpdate()
{
	bool run = false;
	for( const Instance& currentInstance : instancesAndSurfaces )
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
	const size_t AMOUNT_OF_INSTANCES_CONSTANT = instancesAndSurfaces.size();
	for( unsigned int i = 0; i < AMOUNT_OF_INSTANCES_CONSTANT; ++i )
	{
		const size_t AMOUNT_OF_LOGICAL_DEVICES_CONSTANT = instancesAndSurfaces[ i ].logicalDevices.size();
		for( unsigned int j = 0; j < AMOUNT_OF_LOGICAL_DEVICES_CONSTANT; ++j )
		{
			vkDestroySwapchainKHR( instancesAndSurfaces[ i ].logicalDevices[ j ].logicalDevice, 
					instancesAndSurfaces[ i ].logicalDevices[ j ].swapChain, nullptr );
			vkDestroyDevice( instancesAndSurfaces[ i ].logicalDevices[ j ].logicalDevice, nullptr );
		}
		const size_t AMOUNT_OF_SURFACES_CONSTANT = instancesAndSurfaces[ i ].surfaces.size();
		for( unsigned int j = 0; j < AMOUNT_OF_SURFACES_CONSTANT; ++j )
		{
			vkDestroySurfaceKHR( instancesAndSurfaces[ i ].instance,
				instancesAndSurfaces[ i ].surfaces[ j ]->surface, nullptr );
			glfwDestroyWindow( instancesAndSurfaces[ i ].surfaces[ j ]->window );
		}
		DestroyInstance( instancesAndSurfaces[ i ].instance );
	}
	glfwTerminate();
}

