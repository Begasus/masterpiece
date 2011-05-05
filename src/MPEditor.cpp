#include "MPEditor.h"

MPEditor::MPEditor(const BMessage &msg, const BMessenger &msgr, BString windowTitle, int ideaID)
	:	BWindow(BRect(100, 100, 900, 700), windowTitle, B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS, B_CURRENT_WORKSPACE), launcherMessage(msg), launcherMessenger(msgr)
{
	// initialize controls
	BRect r = Bounds();
	r.bottom = r.bottom - 50;
	editorTextView = new BTextView(r, NULL, r, B_FOLLOW_ALL, B_WILL_DRAW);	
	backView = new BView(Bounds(), "backview", B_FOLLOW_ALL, B_WILL_DRAW);
	backView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(backView);
	// gui layout builder
	backView->SetLayout(new BGroupLayout(B_HORIZONTAL, 0.0));
	backView->AddChild(BGridLayoutBuilder()
		.Add(new EditorMenu(), 0, 0)
		.Add(new BScrollView("scroll_editor", editorTextView, B_FOLLOW_ALL_SIDES, 0, false, true, B_FANCY_BORDER), 0, 1)
		.SetInsets(0, 0, 0, 0)
	);
	
	currentideaID = ideaID; // pass current idea id selected to editor window for use
	
	mpdb = OpenSqliteDB(); // open mpdb db
	if(mpdb == NULL) // if db doesn't exist
	{
		eAlert = new ErrorAlert("1.6 Sql Error: Sql DB was not opened properly.");
		eAlert->Launch();
	}
	if(currentideaID != -1) // if id has a real value
	{
		// Pull data from db and populate thoughtview with it
		sqlObject = new SqlObject(mpdb, ideaStatement, "7");
		sqlObject->PrepareSql("select ideatext from ideatable where ideaid = ?");
		sqlObject->BindValue(1, currentideaID);
		sqlObject->StepSql();
		editorTextView->SetText(sqlObject->ReturnText(0));
	}
}
void MPEditor::MessageReceived(BMessage* msg)
{
	BRect r(Bounds());
	switch(msg->what)
	{
		case MENU_NEW_THT: // open another untitled editor window
			tmpEditor = new MPEditor(BMessage(SHOW_LAUNCHER), BMessenger(this), "MasterPiece Editor - untitled", -1);
			tmpEditor->Show();
			break;
		case MENU_EDT_THT: // edit current idea name for editing
			xPos = (r.right - r.left) / 2; // find xpos for window
			yPos = (r.bottom - r.top) / 2; // find ypos for window
			editIdeaName = new EditIdeaName(BMessage(UPDATE_TITLE), BMessenger(this), xPos, yPos, currentideaID);
			editIdeaName->Show(); // show edit idea name window
			break;
		case MENU_SAV_THT: // save current idea progress
			if(currentideaID == -1) // if its untitled insert new thought, then show saveidea to apply a name...
			{
				sqlValue = sqlite3_prepare_v2(mpdb, "insert into ideatable (ideaname, ideatext, ismp) values('untitled', ?, 0)", -1, &ideaStatement, NULL);
				if(sqlValue == SQLITE_OK) // sql statement was prepared properly
				{
					if(sqlite3_bind_text(ideaStatement, 1, editorTextView->Text(), -1, SQLITE_TRANSIENT) == SQLITE_OK) // bind was successful
					{
						sqlite3_step(ideaStatement); // execute insert statement
						sqlite3_finalize(ideaStatement); // finish the statement
						xPos = (r.right - r.left) / 2; // find xpos for window
						yPos = (r.bottom - r.top) / 2; // find ypos for window
						saveIdea = new SaveIdea(BMessage(UPDATE_TITLE), BMessenger(this), xPos, yPos, sqlite3_last_insert_rowid(mpdb));
						saveIdea->Show(); // show save window to name the untitled thought
					}
					else // sql didn't bind
					{
						eAlert = new ErrorAlert("1.9 Sql Error: Sql Bind failed.");
						eAlert->Launch();
					}
				}
				else // sql didn't prepare properly
				{
					eAlert = new ErrorAlert("1.10 Sql Error: Sql Prepare failed.");
					eAlert->Launch();
				}
			}
			else // already exists, just update ideatext and save new information
			{
				sqlValue = sqlite3_prepare_v2(mpdb, "update ideatable set ideatext = ? where ideaid = ?", -1, &ideaStatement, NULL);
				if(sqlValue == SQLITE_OK) // sql statement was prepared properly
				{
					if(sqlite3_bind_text(ideaStatement, 1, editorTextView->Text(), -1, SQLITE_TRANSIENT) == SQLITE_OK) // bind was successful
					{
						if(sqlite3_bind_int(ideaStatement, 2, currentideaID) == SQLITE_OK) // bind was successful
						{
							sqlite3_step(ideaStatement); // execute update statement
							sqlite3_finalize(ideaStatement); // finish the statement
						}
						else
						{
							eAlert = new ErrorAlert("1.12 Sql Error: Sql Bind failed.");
							eAlert->Launch();
						}
					}
					else
					{
						eAlert = new ErrorAlert("1.11 Sql Error: Sql Bind failed.");
						eAlert->Launch();
					}
				}
				else
				{
					eAlert = new ErrorAlert("1.13 Sql Error: Sql Prepare failed.");
					eAlert->Launch();
				}
			}
			break;
		case MENU_PRV_THT: // preview thought in html in webpositive
			printf("save data, export to python html one and open data in preview window or webpositive");
			break;
		case MENU_PUB_THT: // publish thought by opening publish window
			printf("save data, open publish to window, export to python and save as name in publish window");
			break;
		case MENU_HLP_THT: // open help topic window
			printf("open help topic window");
			break;
		case MENU_KEY_THT: // open keyboard reference window
			printf("open keyboard reference window");
			break;
		case MENU_MRK_THT: // open markup reference window
			printf("open markup reference window");
			break;
		case MENU_ABT_THT: // open about window
			printf("open about window");
			break;
		case UPDATE_TITLE: // update title with the name from the saveidea window
			if(msg->FindString("updatetitle", &updateTitle) == B_OK) // updated title exists in variable
			{
				tmpString = "Masterpiece Editor - ";
				tmpString += updateTitle;
				this->SetTitle(tmpString);
			}
			else // 
			{
				eAlert = new ErrorAlert("3.1 Editor Error: Message not found."); // message variable not found
			}
			break;
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}
bool MPEditor::QuitRequested(void)
{
	// on quit, show launcher by sending message
	launcherMessage.MakeEmpty();
	launcherMessage.AddInt64("showLauncher", 1);
	launcherMessenger.SendMessage(&launcherMessage);
	return true;
}
