const main = document.getElementsByTagName("main")[0];

let info;
let currentTrack = 0;

function make(name, ...childrenOrAttributes) {
  let element = document.createElement(name);
  
  let attributes = childrenOrAttributes[0];
  
  if (attributes && Object.getPrototypeOf(attributes) === Object.prototype) {
    for (let key in attributes) {
      if (key == "classes") {
        for (let item of attributes.classes) {
          element.classList.add(item);
        }
      } else {
        element[key] = attributes[key];
      }
    }
    
    element.append(...childrenOrAttributes.slice(1));
  } else {
    element.append(...childrenOrAttributes);
  }
  
  return element;
}

function lowercaseSort(a, b) {
  return a.localeCompare(b, undefined, { sensitivity: "base" });
}

async function load() {
  info = JSON.parse(await (await fetch("info.json")).text());
  showAlbums();
}

function showAlbums() {
  main.replaceChildren();
  
  for (let artist of Object.keys(info).toSorted(lowercaseSort)) {
    main.append(make("h2", artist));
    
    for (let album of Object.keys(info[artist]).toSorted(lowercaseSort)) {
      let element = make("a", { href: "javascript:void(0)" }, make("img", { src: info[artist][album][0].picture, height: 128 }), ` ${album}`);
      
      element.addEventListener("click", () => {
        showAlbum(artist, album);
      });
      
      main.append(make("p", element));
    }
  }
}

function showAlbum(artist, album) {
  main.replaceChildren();
  currentTrack = 0;
  
  let backLink = make("a", { href: "javascript:void(0)" }, "Back");
  
  backLink.addEventListener("click", () => {
    showAlbums();
  });
  
  main.append(make("p", backLink));
  
  main.append(make("h2", `${artist} - ${album}`));
  
  let pictureElement = make("img", { src: info[artist][album][0].picture, height: 256 });
  main.append(make("p", pictureElement));
  
  let titleElement = make("p", `1 - ${info[artist][album][0].title}`);
  main.append(titleElement);
  
  let audioElement = make("audio", { src: info[artist][album][0].file, controls: true });
  
  audioElement.addEventListener("ended", () => {
    playTrack(currentTrack+1);
  });
  
  main.append(make("p", audioElement));
  
  let tracks = make("p");
  
  function playTrack(track) {
    if (info[artist][album][track]) {
      titleElement.innerText = `${track+1} - ${info[artist][album][track].title}`;
      audioElement.src = info[artist][album][track].file;
      pictureElement.src = info[artist][album][track].picture;
      audioElement.play();
      currentTrack = track;
    }
  }
  
  for (let i = 0; i < info[artist][album].length; i++) {
    let track = info[artist][album][i];
    
    let playLink = make("a", { href: "javascript:void(0)" }, `${i+1} - ${track.title}`);
    
    playLink.addEventListener("click", () => {
      playTrack(i);
    });
    
    tracks.append(playLink);
    tracks.append(make("br"));
  }
  
  main.append(tracks);
}

load();
