#include <iostream>
using namespace std;

#include "dispatcher.h"

Dispatcher::Dispatcher(QFrame* centralFrame)
{
	user_ = new User();
	exercise_ = NULL;

	marBackend_ = new MarBackend();
	connect(marBackend_, SIGNAL(gotAudio()),
	        this, SLOT(analyze()));
	connect(marBackend_, SIGNAL(setAttempt(bool)),
	        this, SLOT(setAttempt(bool)));


	string audioFile = "data/sd.wav";
	metro_ = new Metro(0, audioFile);


	centralFrame_ = centralFrame;
	attemptRunningBool_ = false;

	campaign_ = NULL;
}

Dispatcher::~Dispatcher()
{
//	close();
}

bool Dispatcher::close()
{
	if ( closeUser() )
	{
		if (user_ != NULL)
		{
			delete user_;
			user_ = NULL;
		}
		if (campaign_ != NULL)
		{
			delete campaign_;
			campaign_ = NULL;
		}
		if (metro_ != NULL)
		{
			delete metro_;
			metro_ = NULL;
		}
		if (marBackend_ != NULL)
		{
			delete marBackend_;
			marBackend_ = NULL;
		}
		return true;
	}
	return false;
}

bool Dispatcher::closeUser()
{
	if (user_ == NULL)
		return true;
	if ( closeExercise() )
		if ( user_->close() )
		{
			user_->reset();
			return true;
		}
	return false;
}

bool Dispatcher::closeExercise()
{
	if (exercise_ == NULL)
		return true;
	delete exercise_;
	exercise_ = NULL;
	return true;
}

QString Dispatcher::getTitle()
{
	QString title = "Meaws";
	QString next = user_->getName();
	if (!next.isEmpty())
		title.append(QString(" - %1").arg(next));
	return title;
}

QString Dispatcher::getStatus()
{
	if (exercise_ == NULL)
		return "";
	return exercise_->getMessage();
}


// Actions

void Dispatcher::openExercise()
{
	if (exercise_ != NULL)
		delete exercise_;
	exercise_ = ChooseExercise::chooseType();
	if (exercise_ == NULL)
		return;

	QString filename =
	    ChooseExercise::chooseFile(exercise_->exercisesDir());
	if (filename.isEmpty())
		return;

	exercise_->open(filename);
	setupExercise();
}

//zz
void Dispatcher::openCampaign()
{
	QString dir(MEAWS_DIR);
	dir.append("data/campaign");
	QString filename =
	    ChooseExercise::chooseCampaign( dir );
	if (filename.isEmpty())
		return;

	campaign_ = new Campaign(filename);
	exercise_ = campaign_->getNextExercise(-1);
	setupExercise();
}

void Dispatcher::setupExercise()
{
	connect(exercise_, SIGNAL(setupBackend()),
	        this, SLOT(setupBackend()));
	exercise_->setupDisplay(centralFrame_);
	exercise_->addTry();
	updateMain(MEAWS_READY_EXERCISE);
}

void Dispatcher::setupBackend()
{
	if ( exercise_->hasAudio() )
	{
		marBackend_->setBackend( BACKEND_PLAYBACK, true,
		                         exercise_->getFilename() );
	}
	else
	{
		marBackend_->setBackend( exercise_->getBackend(), false,
		                         exercise_->getFilename() );
	}
	marBackend_->setup();
}


void Dispatcher::openAttempt()
{
	QString filename = ChooseExercise::chooseAttempt();
	exercise_->setFilename( qPrintable(filename) );
	marBackend_->setBackend( exercise_->getBackend(), true,
	                         exercise_->getFilename());
	marBackend_->setup();
	marBackend_->start();
}


void Dispatcher::analyze()
{
	if ( !exercise_->hasAudio() )
	{
		if ( marBackend_->analyze() )
		{
			exercise_->displayAnalysis( marBackend_ );
			emit updateMain(MEAWS_UPDATE);
		}
	}
}

void Dispatcher::saveExerciseScore()
{
	user_->saveExercise(exercise_->getExerciseFilename(),
	                    exercise_->getScore());
}

void Dispatcher::saveExerciseAudio()
{
	cout<<"DISPAT save exercise audio"<<endl;
}

void Dispatcher::toggleAttempt()
{
	double score = exercise_->getScore();
	if ( score < 0 )
		setAttempt(!attemptRunningBool_);
	else
	{
		saveExerciseScore();
		delete exercise_;
		exercise_ = campaign_->getNextExercise(score);
		setupExercise();
	}
}

void Dispatcher::setAttempt(bool running)
{
	// if the attempt state has changed
	if (running != attemptRunningBool_)
	{
		if (running)
		{
			attemptRunningBool_ = true;
			marBackend_->start();
			metro_->start();
			updateMain(MEAWS_TRY_RUNNING);
		}
		else
		{
			attemptRunningBool_ = false;
			marBackend_->stop();
			metro_->stop();
			updateMain(MEAWS_TRY_PAUSED);
		}
	}
}

