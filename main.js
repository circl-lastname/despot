const MIN_WIDTH = 600;

let info;

let currentAudio;
let preloadedAudio;

let playerHidden = false;
let queuePreviousScroll = 0;

function lowercaseSort(a, b) {
  return a.localeCompare(b, undefined, { sensitivity: "base" });
}

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

function makeButton(icon, callback) {
  return make("button", { onclick: callback }, make("img", { src: icon, draggable: false }));
}

function makeAlbumCard(artist, album) {
  return make("div", { classes: [ "album" ] },
    make("img", { classes: [ "picture" ], src: info[artist][album][0].picture, width: 200 }),
    make("div", { classes: [ "albumName" ] }, album),
    make("div", { classes: [ "artist" ] }, artist),
    make("div", { classes: [ "buttonDiv" ] },
      //makeButton("assets/list.svg", () => { alert("TODO") }),
      makeButton("assets/add-to-queue.svg", () => { addAlbumToQueue(artist, album) }),
      makeButton("assets/replace-queue.svg", () => { replaceQueueWithAlbum(artist, album) }),
    ),
  );
}

function makeQueueEntry(artist, album, track) {
  let queueEntry = make("div", { classes: [ "queueEntry" ] },
    make("span", { classes: [ "queueEntryTitle" ], onclick: queueEntryClicked }, info[artist][album][track].title),
    //makeButton("assets/up.svg", () => { alert("TODO") }),
    //makeButton("assets/down.svg", () => { alert("TODO") }),
    makeButton("assets/remove.svg", queueEntryRemove),
  );
  
  queueEntry.despot = {
    artist: artist,
    album: album,
    track: track
  };
  
  return queueEntry;
}

async function load() {
  window.addEventListener("resize", () => {
    if (window.innerWidth < MIN_WIDTH && !playerHidden) {
      idMain.classList.add("hideBrowser");
    } else {
      idMain.classList.remove("hideBrowser");
    }
  });
  
  if (window.innerWidth < MIN_WIDTH) {
    idMain.classList.add("hideBrowser");
  }
  
  idShowButton.addEventListener("click", showOrHidePlayer);
  
  info = await (await fetch("info.json")).json();
  
  idProgress.addEventListener("input", () => {
    if (currentAudio) {
      currentAudio.currentTime = idProgress.value;
    }
  });
  
  idPreviousButton.addEventListener("click", previous);
  idPlayButton.addEventListener("click", playOrPause);
  idNextButton.addEventListener("click", next);
  idRemoveAllButton.addEventListener("click", removeAllFromQueue);
  
  displayAlbums();
}

function showOrHidePlayer() {
  if (playerHidden) {
    idMain.classList.remove("hidePlayer");
    idShowButton.firstChild.src = "assets/left.svg";
    
    if (window.innerWidth < MIN_WIDTH) {
      idMain.classList.add("hideBrowser");
    }
    
    playerHidden = false;
  } else {
    idMain.classList.add("hidePlayer");
    idShowButton.firstChild.src = "assets/right.svg";
    
    if (window.innerWidth < MIN_WIDTH) {
      idMain.classList.remove("hideBrowser");
    }
    
    playerHidden = true;
  }
}

function stop() {
  idPicture.src = "assets/no-picture.svg";
  idTitle.innerText = "\xa0";
  idArtist.innerText = "\xa0";
  idProgress.max = 0;
  idProgress.value = 0;
  idPlayButton.firstChild.src = "assets/play.svg";
  
  if (currentAudio) {
    currentAudio.pause();
    currentAudio.removeAttribute("src");
    currentAudio.load();
    currentAudio = undefined;
  }
  
  preloadedAudio = undefined;
}

function playOrPause() {
  if (!currentAudio) {
    if (idQueue.children[0]) {
      playTrack(0, false);
    }
  } else if (currentAudio.paused) {
    currentAudio.play();
  } else {
    currentAudio.pause();
  }
}

function previous() {
  const queue = Array.from(idQueue.children);
  const playing = document.querySelector(".playing");
  const playingQueueTrack = queue.indexOf(playing);
  
  if (queue[playingQueueTrack-1]) {
    playTrack(playingQueueTrack-1, true);
  }
}

function next() {
  const queue = Array.from(idQueue.children);
  const playing = document.querySelector(".playing");
  const playingQueueTrack = queue.indexOf(playing);
  
  if (queue[playingQueueTrack+1]) {
    playTrack(playingQueueTrack+1, true);
  }
}

function queueEntryClicked(e) {
  const queue = Array.from(idQueue.children);
  const selectedQueueTrack = queue.indexOf(e.target.parentElement);
  
  playTrack(selectedQueueTrack, false);
}

function queueEntryRemove(e) {
  // Get the queueEntry class element
  let queueEntry = e.target.parentElement;
  if (queueEntry.tagName == "BUTTON") {
    queueEntry = queueEntry.parentElement;
  }
  
  // Get the queue array, and the queue tracks of the currently playing and clicked
  const queue = Array.from(idQueue.children);
  const playing = document.querySelector(".playing");
  const playingQueueTrack = queue.indexOf(playing);
  const clickedQueueTrack = queue.indexOf(queueEntry);
  
  // If removing currently playing track
  if (playingQueueTrack == clickedQueueTrack) {
    // Try to skip to next track
    if (queue[playingQueueTrack+1]) {
      playTrack(playingQueueTrack+1, false);
    } else {
      // If no more tracks, stop
      stop();
    }
  } else if (playingQueueTrack+1 == clickedQueueTrack) {
    // If removing preloaded track...
    // If there's a track after it
    if (queue[playingQueueTrack+2]) {
      // Preload it
      const nextEntryIdent = queue[playingQueueTrack+2].despot;
      const nextEntryInfo = info[nextEntryIdent.artist][nextEntryIdent.album][nextEntryIdent.track];
      preloadedAudio = new Audio(nextEntryInfo.file);
    } else {
      // Otherwise remove the preloaded audio
      preloadedAudio = undefined;
    }
  }
  
  // Remove the queue entry
  queueEntry.remove();
}

function playTrack(queueTrack, autoscroll) {
  // Get current queue as an array
  const queue = Array.from(idQueue.children);
  
  // Get the current playing queue entry, and queue entries of the audio to play (also metadata), and the one after it
  const playing = document.querySelector(".playing");
  const toPlay = queue[queueTrack];
  const toPlayInfo = info[toPlay.despot.artist][toPlay.despot.album][toPlay.despot.track];
  const toPlayNext = queue[queueTrack+1];
  
  // If there was something playing
  if (playing) {
    // Destroy the currently playing audio
    currentAudio.pause();
    currentAudio.removeAttribute("src");
    currentAudio.load();
    currentAudio = undefined;
    
    // Get the queue track number of what was playing
    const playingQueueTrack = queue.indexOf(playing);
    
    // If the number is one before the one to play
    if (queueTrack == playingQueueTrack+1) {
      // Move the preloaded audio to the current audio
      currentAudio = preloadedAudio;
      preloadedAudio = undefined;
      console.log("Successfully used preloaded audio")
    } else {
      // Otherwise, load it manually
      currentAudio = new Audio(toPlayInfo.file);
    }
    
    // Remove the playing class from the queue entry
    playing.classList.remove("playing");
  } else {
    // If there wasn't something playing, load the audio to play manually
    currentAudio = new Audio(toPlayInfo.file);
  }
  
  // Add the playing class to the queue entry to play
  toPlay.classList.add("playing");
  
  // Show the metadata
  idPicture.src = toPlayInfo.picture;
  idTitle.innerText = toPlayInfo.title;
  idArtist.innerText = toPlay.despot.artist;
  idProgress.value = 0;
  
  // Scroll the queue, if the user hasn't scrolled, if requested
  if (autoscroll) {
    if (queuePreviousScroll == idQueue.scrollTop) {
      toPlay.scrollIntoView({ block: "center" });
    }
    
    queuePreviousScroll = idQueue.scrollTop;
  }
  
  // Try to update duration early
  if (!Number.isNaN(currentAudio.duration)) {
    idProgress.max = currentAudio.duration;
  }
  
  // Set up audio events
  currentAudio.addEventListener("durationchange", () => {
    idProgress.max = currentAudio.duration;
  });
  
  currentAudio.addEventListener("timeupdate", (e) => {
    if (currentAudio == e.target) {
      idProgress.value = currentAudio.currentTime;
    }
  });
  
  currentAudio.addEventListener("play", () => {
    idPlayButton.firstChild.src = "assets/pause.svg";
  });
  
  currentAudio.addEventListener("pause", () => {
    idPlayButton.firstChild.src = "assets/play.svg";
  });
  
  currentAudio.addEventListener("ended", () => {
    next();
  });
  
  // Play the new current audio
  currentAudio.play();
  
  // If there is a queue entry to play next
  if (toPlayNext) {
    // Preload it
    const file = info[toPlayNext.despot.artist][toPlayNext.despot.album][toPlayNext.despot.track].file;
    preloadedAudio = new Audio(file);
  } else {
    preloadedAudio = undefined;
  }
}

function displayAlbums() {
  for (let artist of Object.keys(info).toSorted(lowercaseSort))  {
    for (let album of Object.keys(info[artist]).toSorted(lowercaseSort)) {
      idBrowser.append(makeAlbumCard(artist, album));
    }
  }
}

function addAlbumToQueue(artist, album) {
  for (let i = 0; i < info[artist][album].length; i++) {
    idQueue.append(makeQueueEntry(artist, album, i));
    
    if (!preloadedAudio) {
      preloadedAudio = new Audio(info[artist][album][i].file);
    }
  }
}

function replaceQueueWithAlbum(artist, album) {
  idQueue.replaceChildren();
  
  for (let i = 0; i < info[artist][album].length; i++) {
    idQueue.append(makeQueueEntry(artist, album, i));
  }
  
  let resume = currentAudio && !currentAudio.paused;
  
  stop();
  
  if (resume) {
    playOrPause();
  }
}

function removeAllFromQueue() {
  idQueue.replaceChildren();
  stop();
}

load();
