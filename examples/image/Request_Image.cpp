/**
 * Request Image
 * by Ira Greenberg 
 * 
 * Shows how to use the requestImage() function with preloader animation. 
 * The requestImage() function loads images on a separate thread so that 
 * the sketch does not freeze while they load. It's useful when you are 
 * loading large images. These images are small for a quick download, but 
 * try it with your own huge images to get the full effect. 
 */

int imgCount = 12;
std::vector<PImage*> imgs(imgCount);
float imgW;

// Keeps track of loaded images (true or false)
std::vector<bool> loadStates(imgCount, false);

// For loading animation
float loaderX = 0, loaderY = 0, theta = 0;

void setup() {
  size(640, 360);
  imgW = width / imgCount;

  // Load images asynchronously
  for (int i = 0; i < imgCount; i++){
    // assuming you have nf() equivalent returning std::string
    imgs[i] = requestImage("PT_anim" + nf(i, 4) + ".gif");
  }
}

void draw(){
  background(0);

  // Start loading animation
  runLoaderAni();

  for (int i = 0; i < imgs.size(); i++){
    // Check if individual images are fully loaded
    if (imgs[i] && imgs[i]->width != 0 && imgs[i]->width != -1){
      loadStates[i] = true;
    }
  }

  // When all images are loaded draw them to the screen
  if (checkLoadStates()){
    drawImages();
  }
}

void drawImages() {
  int y = (height - imgs[0]->height) / 2;

  for (int i = 0; i < imgs.size(); i++){
    image(imgs[i], width / imgs.size() * i, y,
          imgs[i]->height, imgs[i]->height);
  }
}

// Loading animation
void runLoaderAni(){
  // Only run when images are loading
  if (!checkLoadStates()){
    ellipse(loaderX, loaderY, 10, 10);

    loaderX += 2;
    loaderY = height/2 + sin(theta) * (height/8);
    theta += PI/22;

    // Reposition ellipse if it goes off the screen
    if (loaderX > width + 5){
      loaderX = -5;
    }
  }
}

// Return true when all images are loaded
bool checkLoadStates(){
  for (int i = 0; i < loadStates.size(); i++){
    if (!loadStates[i]){
      return false;
    }
  }
  return true;
}
